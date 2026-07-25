#include <Pyramid/Graphics/Scene/SceneSerializer.hpp>

#include <Pyramid/Graphics/Resources/ResourceManifest.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>
#include <Pyramid/Graphics/Scene.hpp>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>

namespace Pyramid
{
    namespace
    {
        constexpr std::string_view kHeader = "PYRAMID_SCENE";
        constexpr std::string_view kNone = "-";
        constexpr std::size_t kObjectFieldCount = 25;

        struct ParsedObject
        {
            std::string key;
            std::string name;
            std::string meshKey;
            std::string materialKey;
            Math::Vec3 position;
            Math::Quat rotation;
            Math::Vec3 scale;
            bool visible = true;
            bool castShadows = true;
            bool receiveShadows = true;
            RenderBoundsMode boundsMode = RenderBoundsMode::Automatic;
            Math::Vec3 boundsMin = Math::Vec3(-0.5f);
            Math::Vec3 boundsMax = Math::Vec3(0.5f);
            u32 line = 0;
        };

        void AddDiagnostic(
            std::vector<SceneSerializationDiagnostic>& diagnostics,
            SceneSerializationDiagnosticCode code,
            u32 line,
            std::string key,
            std::string message)
        {
            diagnostics.push_back({code, line, std::move(key), std::move(message)});
        }

        bool IsKeyValid(std::string_view key)
        {
            if (key.empty() || key.size() > 255)
            {
                return false;
            }

            for (const char character : key)
            {
                const unsigned char value = static_cast<unsigned char>(character);
                if (!(std::isalnum(value) || character == '.' || character == '_' ||
                      character == '-' || character == '/'))
                {
                    return false;
                }
            }
            return true;
        }

        std::vector<std::string_view> SplitTabs(std::string_view line)
        {
            std::vector<std::string_view> fields;
            std::size_t start = 0;
            while (true)
            {
                const std::size_t end = line.find('\t', start);
                fields.push_back(line.substr(
                    start,
                    end == std::string_view::npos ? end : end - start));
                if (end == std::string_view::npos)
                {
                    break;
                }
                start = end + 1;
            }
            return fields;
        }

        char HexDigit(u8 value)
        {
            return value < 10 ? static_cast<char>('0' + value)
                              : static_cast<char>('a' + value - 10);
        }

        bool IsSerializableName(std::string_view value)
        {
            return value.size() <= 4096 && value.find('\0') == std::string_view::npos;
        }

        std::string EncodeString(std::string_view value)
        {
            if (value.empty())
            {
                return std::string(kNone);
            }

            std::string encoded;
            encoded.reserve(value.size() * 2);
            for (const unsigned char character : value)
            {
                encoded.push_back(HexDigit(static_cast<u8>(character >> 4U)));
                encoded.push_back(HexDigit(static_cast<u8>(character & 0x0fU)));
            }
            return encoded;
        }

        int HexValue(char character)
        {
            if (character >= '0' && character <= '9') return character - '0';
            if (character >= 'a' && character <= 'f') return character - 'a' + 10;
            if (character >= 'A' && character <= 'F') return character - 'A' + 10;
            return -1;
        }

        bool DecodeString(std::string_view encoded, std::string& output)
        {
            if (encoded == kNone)
            {
                output.clear();
                return true;
            }
            if (encoded.empty() || encoded.size() > 8192 || (encoded.size() % 2) != 0)
            {
                return false;
            }

            std::string decoded;
            decoded.reserve(encoded.size() / 2);
            for (std::size_t index = 0; index < encoded.size(); index += 2)
            {
                const int high = HexValue(encoded[index]);
                const int low = HexValue(encoded[index + 1]);
                if (high < 0 || low < 0)
                {
                    return false;
                }
                const char value = static_cast<char>((high << 4) | low);
                if (value == '\0')
                {
                    return false;
                }
                decoded.push_back(value);
            }
            output = std::move(decoded);
            return true;
        }

        bool ParseUnsigned(std::string_view text, u32& value)
        {
            if (text.empty()) return false;
            u64 parsed = 0;
            const auto result = std::from_chars(
                text.data(), text.data() + text.size(), parsed, 10);
            if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
                parsed > std::numeric_limits<u32>::max())
            {
                return false;
            }
            value = static_cast<u32>(parsed);
            return true;
        }

        bool ParseFloat(std::string_view text, f32& value)
        {
            if (text.empty()) return false;
            f32 parsed = 0.0f;
            const auto result = std::from_chars(
                text.data(), text.data() + text.size(), parsed,
                std::chars_format::general);
            if (result.ec != std::errc{} || result.ptr != text.data() + text.size())
            {
                return false;
            }
            value = parsed;
            return true;
        }

        bool ParseBool(std::string_view text, bool& value)
        {
            if (text == "0")
            {
                value = false;
                return true;
            }
            if (text == "1")
            {
                value = true;
                return true;
            }
            return false;
        }

        bool IsFinite(const Math::Vec3& value)
        {
            return std::isfinite(value.x) && std::isfinite(value.y) &&
                std::isfinite(value.z);
        }

        bool IsFinite(const Math::Quat& value)
        {
            return std::isfinite(value.x) && std::isfinite(value.y) &&
                std::isfinite(value.z) && std::isfinite(value.w);
        }

        std::string MakeObjectKey(std::size_t index)
        {
            std::ostringstream stream;
            stream.imbue(std::locale::classic());
            stream << "object/" << std::setfill('0') << std::setw(6) << index;
            return stream.str();
        }

        const ResourceManifestEntry* FindManifestEntry(
            const ResourceManifest& manifest,
            std::string_view key)
        {
            const auto& entries = manifest.GetEntries();
            const auto iterator = std::find_if(
                entries.begin(), entries.end(),
                [key](const ResourceManifestEntry& entry)
                {
                    return entry.key == key;
                });
            return iterator == entries.end() ? nullptr : &*iterator;
        }

        std::string FindManifestKey(
            const ResourceManifest& manifest,
            ResourceType type,
            u64 high,
            u64 low,
            u32 generation)
        {
            std::string selected;
            for (const auto& entry : manifest.GetEntries())
            {
                if (entry.type != type || entry.assetIdHigh != high ||
                    entry.assetIdLow != low || entry.generation != generation)
                {
                    continue;
                }
                if (selected.empty() || entry.key < selected)
                {
                    selected = entry.key;
                }
            }
            return selected;
        }

        MeshHandle GetObjectMeshHandle(
            const RenderObject& object,
            const ResourceRegistry& registry)
        {
            if (object.mesh)
            {
                const MeshHandle handle = registry.GetHandle(object.mesh->GetAssetId());
                return registry.Resolve(handle) == object.mesh ? handle : MeshHandle{};
            }
            return registry.Resolve(object.meshHandle) ? object.meshHandle : MeshHandle{};
        }

        MaterialHandle GetObjectMaterialHandle(
            const RenderObject& object,
            const ResourceRegistry& registry)
        {
            if (object.material)
            {
                const MaterialHandle handle = registry.GetHandle(object.material->GetAssetId());
                return registry.Resolve(handle) == object.material ? handle : MaterialHandle{};
            }
            return registry.Resolve(object.materialHandle) ? object.materialHandle : MaterialHandle{};
        }

        template <typename HandleType>
        std::string FindManifestKey(
            const ResourceManifest& manifest,
            ResourceType type,
            HandleType handle)
        {
            if (!handle)
            {
                return {};
            }
            const auto id = handle.GetAssetId();
            return FindManifestKey(
                manifest, type, id.high, id.low, handle.GetGeneration());
        }

        template <typename HandleType, typename AssetIdType>
        bool ValidateManifestResource(
            std::string_view resourceKey,
            ResourceType expectedType,
            const ResourceManifest& manifest,
            const ResourceRegistry& registry,
            u32 line,
            std::string_view objectKey,
            HandleType& output,
            SceneDeserializationResult& result)
        {
            output = {};
            if (resourceKey == kNone)
            {
                return true;
            }
            if (!IsKeyValid(resourceKey))
            {
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::MissingManifestResource,
                    line,
                    std::string(objectKey),
                    "resource manifest key is invalid");
                return false;
            }

            const ResourceManifestEntry* entry = FindManifestEntry(manifest, resourceKey);
            if (!entry)
            {
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::MissingManifestResource,
                    line,
                    std::string(objectKey),
                    "resource manifest key was not found: " + std::string(resourceKey));
                return false;
            }
            if (entry->type != expectedType)
            {
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::ResourceTypeMismatch,
                    line,
                    std::string(objectKey),
                    "resource manifest key has the wrong resource type: " +
                        std::string(resourceKey));
                return false;
            }

            const AssetIdType assetId{entry->assetIdHigh, entry->assetIdLow};
            output = HandleType::FromParts(assetId, entry->generation);
            if (registry.Resolve(output))
            {
                return true;
            }

            const HandleType current = registry.GetHandle(assetId);
            if (!current)
            {
                ++result.missingAssets;
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::MissingAsset,
                    line,
                    std::string(objectKey),
                    "resource is not resident: " + std::string(resourceKey));
            }
            else if (current.GetGeneration() != output.GetGeneration())
            {
                ++result.staleGenerations;
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::StaleGeneration,
                    line,
                    std::string(objectKey),
                    "resource generation is stale: " + std::string(resourceKey));
            }
            else
            {
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::UnresolvedResource,
                    line,
                    std::string(objectKey),
                    "resource could not be resolved: " + std::string(resourceKey));
            }
            return false;
        }
    }

    SceneSerializationResult SceneSerializer::Serialize(
        const Scene& scene,
        const ResourceManifest& manifest,
        const ResourceRegistry& registry)
    {
        SceneSerializationResult result;
        if (!IsSerializableName(scene.GetName()))
        {
            AddDiagnostic(
                result.diagnostics,
                SceneSerializationDiagnosticCode::InvalidNameEncoding,
                0,
                {},
                "scene name is too long or contains a null byte");
            return result;
        }

        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::setprecision(std::numeric_limits<f32>::max_digits10);
        stream << kHeader << '\t' << kSceneSerializationVersion << '\n';
        stream << "scene\t" << EncodeString(scene.GetName()) << '\n';

        const auto& objects = scene.GetRenderObjects();
        for (std::size_t index = 0; index < objects.size(); ++index)
        {
            const std::string objectKey = MakeObjectKey(index);
            const auto& object = objects[index];
            if (!object)
            {
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::MalformedRecord,
                    0,
                    objectKey,
                    "scene contains a null render object");
                continue;
            }

            if (!IsSerializableName(object->name))
            {
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::InvalidNameEncoding,
                    0,
                    objectKey,
                    "render-object name is too long or contains a null byte");
                continue;
            }

            std::string meshKey(kNone);
            if (object->mesh || object->meshHandle)
            {
                const MeshHandle handle = GetObjectMeshHandle(*object, registry);
                if (!handle)
                {
                    AddDiagnostic(
                        result.diagnostics,
                        SceneSerializationDiagnosticCode::UnresolvedResource,
                        0,
                        objectKey,
                        "mesh is not a live registry resource");
                    continue;
                }
                meshKey = FindManifestKey(manifest, ResourceType::Mesh, handle);
                if (meshKey.empty())
                {
                    AddDiagnostic(
                        result.diagnostics,
                        SceneSerializationDiagnosticCode::MissingManifestResource,
                        0,
                        objectKey,
                        "mesh handle has no matching resource-manifest key");
                    continue;
                }
            }

            std::string materialKey(kNone);
            if (object->material || object->materialHandle)
            {
                const MaterialHandle handle = GetObjectMaterialHandle(*object, registry);
                if (!handle)
                {
                    AddDiagnostic(
                        result.diagnostics,
                        SceneSerializationDiagnosticCode::UnresolvedResource,
                        0,
                        objectKey,
                        "material is not a live registry resource");
                    continue;
                }
                materialKey = FindManifestKey(manifest, ResourceType::Material, handle);
                if (materialKey.empty())
                {
                    AddDiagnostic(
                        result.diagnostics,
                        SceneSerializationDiagnosticCode::MissingManifestResource,
                        0,
                        objectKey,
                        "material handle has no matching resource-manifest key");
                    continue;
                }
            }

            Math::Vec3 boundsMin;
            Math::Vec3 boundsMax;
            object->GetLocalBounds(boundsMin, boundsMax);
            if (!IsFinite(object->position) || !IsFinite(object->rotation) ||
                !IsFinite(object->scale) || !IsFinite(boundsMin) || !IsFinite(boundsMax))
            {
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::NonFiniteValue,
                    0,
                    objectKey,
                    "render object contains a non-finite transform or bounds value");
                continue;
            }
            if (Math::IsZero(object->rotation.LengthSquared()))
            {
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::InvalidRotation,
                    0,
                    objectKey,
                    "render-object rotation quaternion has zero length");
                continue;
            }

            const Math::Quat rotation = object->rotation.Normalized();
            stream << "object\t" << objectKey << '\t' << EncodeString(object->name)
                   << '\t' << meshKey << '\t' << materialKey
                   << '\t' << object->position.x << '\t' << object->position.y
                   << '\t' << object->position.z << '\t' << rotation.x
                   << '\t' << rotation.y << '\t' << rotation.z << '\t' << rotation.w
                   << '\t' << object->scale.x << '\t' << object->scale.y
                   << '\t' << object->scale.z << '\t' << (object->visible ? 1 : 0)
                   << '\t' << (object->castShadows ? 1 : 0)
                   << '\t' << (object->receiveShadows ? 1 : 0)
                   << '\t' << (object->boundsMode == RenderBoundsMode::Manual ? "manual" : "auto")
                   << '\t' << boundsMin.x << '\t' << boundsMin.y << '\t' << boundsMin.z
                   << '\t' << boundsMax.x << '\t' << boundsMax.y << '\t' << boundsMax.z
                   << '\n';
            ++result.serializedObjects;
        }

        if (result.diagnostics.empty())
        {
            result.text = stream.str();
        }
        return result;
    }

    SceneDeserializationResult SceneSerializer::Deserialize(
        std::string_view text,
        const ResourceManifest& manifest,
        const ResourceRegistry& registry)
    {
        SceneDeserializationResult result;
        std::vector<ParsedObject> parsedObjects;
        std::unordered_set<std::string> objectKeys;
        std::string sceneName;
        bool sceneRecordFound = false;

        std::size_t offset = 0;
        u32 lineNumber = 0;
        while (offset <= text.size())
        {
            const std::size_t end = text.find('\n', offset);
            std::string_view line = text.substr(
                offset,
                end == std::string_view::npos ? end : end - offset);
            ++lineNumber;
            if (!line.empty() && line.back() == '\r')
            {
                line.remove_suffix(1);
            }

            if (lineNumber == 1)
            {
                const auto fields = SplitTabs(line);
                u32 version = 0;
                if (fields.size() != 2 || fields[0] != kHeader ||
                    !ParseUnsigned(fields[1], version))
                {
                    AddDiagnostic(
                        result.diagnostics,
                        SceneSerializationDiagnosticCode::InvalidHeader,
                        lineNumber,
                        {},
                        "invalid scene header");
                }
                else if (version != kSceneSerializationVersion)
                {
                    AddDiagnostic(
                        result.diagnostics,
                        SceneSerializationDiagnosticCode::UnsupportedVersion,
                        lineNumber,
                        {},
                        "unsupported scene version");
                }
            }
            else if (!line.empty())
            {
                const auto fields = SplitTabs(line);
                if (fields[0] == "scene")
                {
                    if (fields.size() != 2)
                    {
                        AddDiagnostic(
                            result.diagnostics,
                            SceneSerializationDiagnosticCode::MalformedRecord,
                            lineNumber,
                            {},
                            "scene record must contain exactly two fields");
                    }
                    else if (sceneRecordFound)
                    {
                        AddDiagnostic(
                            result.diagnostics,
                            SceneSerializationDiagnosticCode::DuplicateSceneRecord,
                            lineNumber,
                            {},
                            "scene record appears more than once");
                    }
                    else if (!DecodeString(fields[1], sceneName))
                    {
                        AddDiagnostic(
                            result.diagnostics,
                            SceneSerializationDiagnosticCode::InvalidNameEncoding,
                            lineNumber,
                            {},
                            "scene name encoding is invalid");
                    }
                    else
                    {
                        sceneRecordFound = true;
                    }
                }
                else if (fields[0] == "object")
                {
                    if (fields.size() != kObjectFieldCount)
                    {
                        AddDiagnostic(
                            result.diagnostics,
                            SceneSerializationDiagnosticCode::MalformedRecord,
                            lineNumber,
                            fields.size() > 1 ? std::string(fields[1]) : std::string{},
                            "object record has the wrong field count");
                    }
                    else
                    {
                        ParsedObject parsed;
                        parsed.key = std::string(fields[1]);
                        parsed.meshKey = std::string(fields[3]);
                        parsed.materialKey = std::string(fields[4]);
                        parsed.line = lineNumber;

                        bool valid = true;
                        if (!IsKeyValid(parsed.key))
                        {
                            AddDiagnostic(
                                result.diagnostics,
                                SceneSerializationDiagnosticCode::InvalidObjectKey,
                                lineNumber,
                                parsed.key,
                                "object key is invalid");
                            valid = false;
                        }
                        else if (!objectKeys.insert(parsed.key).second)
                        {
                            AddDiagnostic(
                                result.diagnostics,
                                SceneSerializationDiagnosticCode::DuplicateObjectKey,
                                lineNumber,
                                parsed.key,
                                "object key is duplicated");
                            valid = false;
                        }
                        if (!DecodeString(fields[2], parsed.name))
                        {
                            AddDiagnostic(
                                result.diagnostics,
                                SceneSerializationDiagnosticCode::InvalidNameEncoding,
                                lineNumber,
                                parsed.key,
                                "object name encoding is invalid");
                            valid = false;
                        }

                        f32 values[16] = {};
                        const std::size_t numericFields[] = {
                            5, 6, 7, 8, 9, 10, 11, 12,
                            13, 14, 19, 20, 21, 22, 23, 24};
                        for (std::size_t index = 0; index < 16; ++index)
                        {
                            if (!ParseFloat(fields[numericFields[index]], values[index]))
                            {
                                AddDiagnostic(
                                    result.diagnostics,
                                    SceneSerializationDiagnosticCode::InvalidNumber,
                                    lineNumber,
                                    parsed.key,
                                    "object contains an invalid numeric field");
                                valid = false;
                                break;
                            }
                        }

                        parsed.position = Math::Vec3(values[0], values[1], values[2]);
                        parsed.rotation = Math::Quat(values[3], values[4], values[5], values[6]);
                        parsed.scale = Math::Vec3(values[7], values[8], values[9]);
                        parsed.boundsMin = Math::Vec3(values[10], values[11], values[12]);
                        parsed.boundsMax = Math::Vec3(values[13], values[14], values[15]);

                        if (valid && (!IsFinite(parsed.position) || !IsFinite(parsed.rotation) ||
                            !IsFinite(parsed.scale) || !IsFinite(parsed.boundsMin) ||
                            !IsFinite(parsed.boundsMax)))
                        {
                            AddDiagnostic(
                                result.diagnostics,
                                SceneSerializationDiagnosticCode::NonFiniteValue,
                                lineNumber,
                                parsed.key,
                                "object transform or bounds contains a non-finite value");
                            valid = false;
                        }
                        if (valid && Math::IsZero(parsed.rotation.LengthSquared()))
                        {
                            AddDiagnostic(
                                result.diagnostics,
                                SceneSerializationDiagnosticCode::InvalidRotation,
                                lineNumber,
                                parsed.key,
                                "object rotation quaternion has zero length");
                            valid = false;
                        }
                        if (!ParseBool(fields[15], parsed.visible) ||
                            !ParseBool(fields[16], parsed.castShadows) ||
                            !ParseBool(fields[17], parsed.receiveShadows))
                        {
                            AddDiagnostic(
                                result.diagnostics,
                                SceneSerializationDiagnosticCode::InvalidBoolean,
                                lineNumber,
                                parsed.key,
                                "object visibility or shadow flag is invalid");
                            valid = false;
                        }
                        if (fields[18] == "manual")
                        {
                            parsed.boundsMode = RenderBoundsMode::Manual;
                        }
                        else if (fields[18] == "auto")
                        {
                            parsed.boundsMode = RenderBoundsMode::Automatic;
                        }
                        else
                        {
                            AddDiagnostic(
                                result.diagnostics,
                                SceneSerializationDiagnosticCode::InvalidBoundsMode,
                                lineNumber,
                                parsed.key,
                                "object bounds mode must be auto or manual");
                            valid = false;
                        }

                        if (valid)
                        {
                            parsed.rotation.Normalize();
                            parsedObjects.push_back(std::move(parsed));
                        }
                    }
                }
                else
                {
                    AddDiagnostic(
                        result.diagnostics,
                        SceneSerializationDiagnosticCode::MalformedRecord,
                        lineNumber,
                        {},
                        "unknown scene record type");
                }
            }

            if (end == std::string_view::npos)
            {
                break;
            }
            offset = end + 1;
        }

        if (!sceneRecordFound)
        {
            AddDiagnostic(
                result.diagnostics,
                SceneSerializationDiagnosticCode::MissingSceneRecord,
                0,
                {},
                "scene record is missing");
        }
        if (!result.diagnostics.empty())
        {
            return result;
        }

        std::vector<std::shared_ptr<RenderObject>> restoredObjects;
        restoredObjects.reserve(parsedObjects.size());
        for (const ParsedObject& parsed : parsedObjects)
        {
            MeshHandle meshHandle;
            MaterialHandle materialHandle;
            const bool meshValid = ValidateManifestResource<MeshHandle, MeshAssetId>(
                parsed.meshKey,
                ResourceType::Mesh,
                manifest,
                registry,
                parsed.line,
                parsed.key,
                meshHandle,
                result);
            const bool materialValid = ValidateManifestResource<MaterialHandle, MaterialAssetId>(
                parsed.materialKey,
                ResourceType::Material,
                manifest,
                registry,
                parsed.line,
                parsed.key,
                materialHandle,
                result);
            if (!meshValid || !materialValid)
            {
                continue;
            }

            auto object = std::make_shared<RenderObject>();
            object->name = parsed.name;
            object->position = parsed.position;
            object->rotation = parsed.rotation;
            object->scale = parsed.scale;
            object->visible = parsed.visible;
            object->castShadows = parsed.castShadows;
            object->receiveShadows = parsed.receiveShadows;

            if (meshHandle && !object->SetMeshHandle(meshHandle, registry))
            {
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::UnresolvedResource,
                    parsed.line,
                    parsed.key,
                    "mesh handle could not be assigned");
                continue;
            }
            if (materialHandle && !object->SetMaterialHandle(materialHandle, registry))
            {
                AddDiagnostic(
                    result.diagnostics,
                    SceneSerializationDiagnosticCode::UnresolvedResource,
                    parsed.line,
                    parsed.key,
                    "material handle could not be assigned");
                continue;
            }

            const Math::Vec3 canonicalMin(
                Math::Min(parsed.boundsMin.x, parsed.boundsMax.x),
                Math::Min(parsed.boundsMin.y, parsed.boundsMax.y),
                Math::Min(parsed.boundsMin.z, parsed.boundsMax.z));
            const Math::Vec3 canonicalMax(
                Math::Max(parsed.boundsMin.x, parsed.boundsMax.x),
                Math::Max(parsed.boundsMin.y, parsed.boundsMax.y),
                Math::Max(parsed.boundsMin.z, parsed.boundsMax.z));
            if (parsed.boundsMode == RenderBoundsMode::Manual)
            {
                object->SetLocalBounds(canonicalMin, canonicalMax);
            }
            else
            {
                object->localBoundsMin = canonicalMin;
                object->localBoundsMax = canonicalMax;
                object->UseAutomaticBounds();
            }
            restoredObjects.push_back(std::move(object));
        }

        if (!result.diagnostics.empty())
        {
            return result;
        }

        auto restoredScene = std::make_shared<Scene>(sceneName);
        for (const auto& object : restoredObjects)
        {
            restoredScene->AddRenderObject(object);
        }
        result.restoredObjects = static_cast<u32>(restoredObjects.size());
        result.scene = std::move(restoredScene);
        return result;
    }
}
