#include <Pyramid/Graphics/Scene/SceneSerializer.hpp>

#include <Pyramid/Graphics/Resources/ResourceManifest.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>
#include <Pyramid/Graphics/Scene.hpp>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace Pyramid
{
    namespace
    {
        constexpr std::string_view kHeader = "PYRAMID_SCENE";
        constexpr std::string_view kNone = "none";
        constexpr std::size_t kMaximumNameBytes = 4096;

        struct ParsedEntity
        {
            EntityId id;
            EntityId parent;
            std::string name;
            bool visible = true;
            TransformComponent transform;
            std::optional<MeshRendererComponent> meshRenderer;
            std::optional<LightComponent> light;
            u32 sourceLine = 0;
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

        std::vector<std::string_view> SplitTabs(std::string_view line)
        {
            std::vector<std::string_view> fields;
            std::size_t begin = 0;
            while (begin <= line.size())
            {
                const std::size_t end = line.find('\t', begin);
                if (end == std::string_view::npos)
                {
                    fields.push_back(line.substr(begin));
                    break;
                }
                fields.push_back(line.substr(begin, end - begin));
                begin = end + 1;
            }
            return fields;
        }

        bool ParseUnsigned(std::string_view text, u32& value)
        {
            const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
            return result.ec == std::errc{} && result.ptr == text.data() + text.size();
        }

        bool ParseFloat(std::string_view text, f32& value)
        {
            const auto result = std::from_chars(
                text.data(), text.data() + text.size(), value, std::chars_format::general);
            return result.ec == std::errc{} && result.ptr == text.data() + text.size() &&
                std::isfinite(value);
        }

        bool ParseBoolean(std::string_view text, bool& value)
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

        bool IsSerializableName(const std::string& value)
        {
            return value.size() <= kMaximumNameBytes &&
                value.find('\0') == std::string::npos;
        }

        char HexDigit(u8 value)
        {
            return value < 10 ? static_cast<char>('0' + value) :
                static_cast<char>('a' + value - 10);
        }

        int HexValue(char value)
        {
            if (value >= '0' && value <= '9') return value - '0';
            if (value >= 'a' && value <= 'f') return value - 'a' + 10;
            if (value >= 'A' && value <= 'F') return value - 'A' + 10;
            return -1;
        }

        std::string EncodeString(const std::string& value)
        {
            if (value.empty())
            {
                return "-";
            }
            std::string output;
            output.reserve(value.size() * 2);
            for (unsigned char byte : value)
            {
                output.push_back(HexDigit(static_cast<u8>(byte >> 4U)));
                output.push_back(HexDigit(static_cast<u8>(byte & 0x0FU)));
            }
            return output;
        }

        bool DecodeString(std::string_view encoded, std::string& output)
        {
            if (encoded == "-")
            {
                output.clear();
                return true;
            }
            if ((encoded.size() % 2U) != 0 || encoded.size() / 2U > kMaximumNameBytes)
            {
                return false;
            }
            std::string decoded;
            decoded.reserve(encoded.size() / 2U);
            for (std::size_t index = 0; index < encoded.size(); index += 2)
            {
                const int high = HexValue(encoded[index]);
                const int low = HexValue(encoded[index + 1]);
                if (high < 0 || low < 0)
                {
                    return false;
                }
                decoded.push_back(static_cast<char>((high << 4) | low));
            }
            if (decoded.find('\0') != std::string::npos)
            {
                return false;
            }
            output = std::move(decoded);
            return true;
        }

        const ResourceManifestEntry* FindManifestEntry(
            const ResourceManifest& manifest,
            std::string_view key)
        {
            for (const auto& entry : manifest.GetEntries())
            {
                if (entry.key == key)
                {
                    return &entry;
                }
            }
            return nullptr;
        }

        template <typename HandleType>
        std::string FindManifestKey(
            const ResourceManifest& manifest,
            ResourceType type,
            HandleType handle)
        {
            if (!handle)
            {
                return std::string(kNone);
            }
            const auto assetId = handle.GetAssetId();
            std::string selected;
            for (const auto& entry : manifest.GetEntries())
            {
                if (entry.type == type && entry.assetIdHigh == assetId.high &&
                    entry.assetIdLow == assetId.low &&
                    entry.generation == handle.GetGeneration() &&
                    (selected.empty() || entry.key < selected))
                {
                    selected = entry.key;
                }
            }
            return selected;
        }

        bool RestoreMesh(
            std::string_view key,
            const ResourceManifest& manifest,
            const ResourceRegistry& registry,
            MeshHandle& output,
            SceneDeserializationResult& result,
            u32 line,
            std::string_view entityKey)
        {
            if (key == kNone)
            {
                output = {};
                return true;
            }
            const ResourceManifestEntry* entry = FindManifestEntry(manifest, key);
            if (!entry)
            {
                AddDiagnostic(result.diagnostics,
                    SceneSerializationDiagnosticCode::MissingManifestResource,
                    line, std::string(entityKey), "mesh manifest key does not exist: " + std::string(key));
                return false;
            }
            if (entry->type != ResourceType::Mesh)
            {
                AddDiagnostic(result.diagnostics,
                    SceneSerializationDiagnosticCode::ResourceTypeMismatch,
                    line, std::string(entityKey), "manifest key is not a mesh: " + std::string(key));
                return false;
            }
            output = manifest.GetMeshHandle(key);
            if (registry.IsAlive(output))
            {
                return true;
            }
            const MeshHandle current = registry.GetHandle(output.GetAssetId());
            if (!current)
            {
                ++result.missingAssets;
                AddDiagnostic(result.diagnostics, SceneSerializationDiagnosticCode::MissingAsset,
                    line, std::string(entityKey), "mesh is not resident: " + std::string(key));
            }
            else
            {
                ++result.staleGenerations;
                AddDiagnostic(result.diagnostics, SceneSerializationDiagnosticCode::StaleGeneration,
                    line, std::string(entityKey), "mesh generation is stale: " + std::string(key));
            }
            return false;
        }

        bool RestoreMaterial(
            std::string_view key,
            const ResourceManifest& manifest,
            const ResourceRegistry& registry,
            MaterialHandle& output,
            SceneDeserializationResult& result,
            u32 line,
            std::string_view entityKey)
        {
            if (key == kNone)
            {
                output = {};
                return true;
            }
            const ResourceManifestEntry* entry = FindManifestEntry(manifest, key);
            if (!entry)
            {
                AddDiagnostic(result.diagnostics,
                    SceneSerializationDiagnosticCode::MissingManifestResource,
                    line, std::string(entityKey), "material manifest key does not exist: " + std::string(key));
                return false;
            }
            if (entry->type != ResourceType::Material)
            {
                AddDiagnostic(result.diagnostics,
                    SceneSerializationDiagnosticCode::ResourceTypeMismatch,
                    line, std::string(entityKey), "manifest key is not a material: " + std::string(key));
                return false;
            }
            output = manifest.GetMaterialHandle(key);
            if (registry.IsAlive(output))
            {
                return true;
            }
            const MaterialHandle current = registry.GetHandle(output.GetAssetId());
            if (!current)
            {
                ++result.missingAssets;
                AddDiagnostic(result.diagnostics, SceneSerializationDiagnosticCode::MissingAsset,
                    line, std::string(entityKey), "material is not resident: " + std::string(key));
            }
            else
            {
                ++result.staleGenerations;
                AddDiagnostic(result.diagnostics, SceneSerializationDiagnosticCode::StaleGeneration,
                    line, std::string(entityKey), "material generation is stale: " + std::string(key));
            }
            return false;
        }

        std::string EntityKey(EntityId id)
        {
            return "entity/" + id.ToString();
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
            AddDiagnostic(result.diagnostics,
                SceneSerializationDiagnosticCode::InvalidNameEncoding,
                0, {}, "scene name is too long or contains a null byte");
            return result;
        }

        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::setprecision(std::numeric_limits<f32>::max_digits10);
        stream << kHeader << '\t' << kSceneSerializationVersion << '\n';
        stream << "scene\t" << EncodeString(scene.GetName()) << '\n';

        for (const Entity& entity : scene.GetEntities())
        {
            const TransformComponent* transform = entity.GetTransform();
            if (!transform || !IsSerializableName(entity.GetName()) ||
                !IsFinite(transform->position) || !IsFinite(transform->rotation) ||
                !IsFinite(transform->scale))
            {
                AddDiagnostic(result.diagnostics,
                    SceneSerializationDiagnosticCode::NonFiniteValue,
                    0, EntityKey(entity.GetId()),
                    "entity contains an invalid name or non-finite transform");
                continue;
            }
            if (Math::IsZero(transform->rotation.LengthSquared()))
            {
                AddDiagnostic(result.diagnostics,
                    SceneSerializationDiagnosticCode::InvalidRotation,
                    0, EntityKey(entity.GetId()), "entity rotation has zero length");
                continue;
            }
            const Math::Quat rotation = transform->rotation.Normalized();
            const Entity parent = entity.GetParent();
            stream << "entity\t" << entity.GetId().ToString() << '\t'
                   << (parent ? parent.GetId().ToString() : std::string(kNone)) << '\t'
                   << EncodeString(entity.GetName()) << '\t' << (entity.IsVisible() ? 1 : 0)
                   << '\t' << transform->position.x << '\t' << transform->position.y
                   << '\t' << transform->position.z << '\t' << rotation.x << '\t'
                   << rotation.y << '\t' << rotation.z << '\t' << rotation.w << '\t'
                   << transform->scale.x << '\t' << transform->scale.y << '\t'
                   << transform->scale.z << '\n';
            ++result.serializedEntities;

            if (const MeshRendererComponent* renderer = entity.GetMeshRenderer())
            {
                const std::string meshKey =
                    FindManifestKey(manifest, ResourceType::Mesh, renderer->mesh);
                const std::string materialKey =
                    FindManifestKey(manifest, ResourceType::Material, renderer->material);
                if ((renderer->mesh && meshKey.empty()) ||
                    (renderer->material && materialKey.empty()) ||
                    (renderer->mesh && !registry.IsAlive(renderer->mesh)) ||
                    (renderer->material && !registry.IsAlive(renderer->material)))
                {
                    AddDiagnostic(result.diagnostics,
                        SceneSerializationDiagnosticCode::UnresolvedResource,
                        0, EntityKey(entity.GetId()),
                        "mesh renderer references a missing/stale resource or manifest key");
                    continue;
                }
                Math::Vec3 minimum = renderer->localBoundsMin;
                Math::Vec3 maximum = renderer->localBoundsMax;
                if (!IsFinite(minimum) || !IsFinite(maximum))
                {
                    AddDiagnostic(result.diagnostics,
                        SceneSerializationDiagnosticCode::NonFiniteValue,
                        0, EntityKey(entity.GetId()), "mesh-renderer bounds are non-finite");
                    continue;
                }
                stream << "mesh_renderer\t" << entity.GetId().ToString() << '\t'
                       << (meshKey.empty() ? std::string(kNone) : meshKey) << '\t'
                       << (materialKey.empty() ? std::string(kNone) : materialKey) << '\t'
                       << (renderer->visible ? 1 : 0) << '\t'
                       << (renderer->castShadows ? 1 : 0) << '\t'
                       << (renderer->receiveShadows ? 1 : 0) << '\t'
                       << (renderer->boundsMode == RenderBoundsMode::Manual ? "manual" : "auto")
                       << '\t' << minimum.x << '\t' << minimum.y << '\t' << minimum.z
                       << '\t' << maximum.x << '\t' << maximum.y << '\t' << maximum.z << '\n';
                ++result.serializedObjects;
            }

            if (const LightComponent* light = entity.GetLight())
            {
                if (!IsFinite(light->localDirection) || !IsFinite(light->color) ||
                    !std::isfinite(light->intensity) || !std::isfinite(light->range) ||
                    !std::isfinite(light->innerConeAngle) ||
                    !std::isfinite(light->outerConeAngle) ||
                    !std::isfinite(light->shadowBias))
                {
                    AddDiagnostic(result.diagnostics,
                        SceneSerializationDiagnosticCode::NonFiniteValue,
                        0, EntityKey(entity.GetId()), "light contains non-finite values");
                    continue;
                }
                stream << "light\t" << entity.GetId().ToString() << '\t'
                       << static_cast<u32>(light->type) << '\t'
                       << light->localDirection.x << '\t' << light->localDirection.y << '\t'
                       << light->localDirection.z << '\t' << light->color.x << '\t'
                       << light->color.y << '\t' << light->color.z << '\t'
                       << light->intensity << '\t' << light->range << '\t'
                       << light->innerConeAngle << '\t' << light->outerConeAngle << '\t'
                       << (light->castShadows ? 1 : 0) << '\t' << light->shadowBias << '\t'
                       << light->shadowMapSize << '\t' << (light->enabled ? 1 : 0) << '\n';
                ++result.serializedLights;
            }
        }

        const Entity primary = scene.GetPrimaryLightEntity();
        stream << "primary_light\t"
               << (primary ? primary.GetId().ToString() : std::string(kNone)) << '\n';

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
        std::vector<ParsedEntity> parsed;
        std::unordered_map<EntityId, std::size_t, EntityIdHash> indices;
        std::optional<std::string> sceneName;
        EntityId primaryLight;
        bool primarySeen = false;
        bool headerSeen = false;

        std::size_t lineStart = 0;
        u32 lineNumber = 0;
        while (lineStart <= text.size())
        {
            const std::size_t lineEnd = text.find('\n', lineStart);
            std::string_view line = lineEnd == std::string_view::npos
                ? text.substr(lineStart)
                : text.substr(lineStart, lineEnd - lineStart);
            if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
            ++lineNumber;
            if (!line.empty())
            {
                const auto fields = SplitTabs(line);
                if (!headerSeen)
                {
                    headerSeen = true;
                    u32 version = 0;
                    if (fields.size() != 2 || fields[0] != kHeader ||
                        !ParseUnsigned(fields[1], version))
                    {
                        AddDiagnostic(result.diagnostics,
                            SceneSerializationDiagnosticCode::InvalidHeader,
                            lineNumber, {}, "invalid scene header");
                    }
                    else if (version != kSceneSerializationVersion)
                    {
                        AddDiagnostic(result.diagnostics,
                            SceneSerializationDiagnosticCode::UnsupportedVersion,
                            lineNumber, {}, "unsupported scene version");
                    }
                }
                else if (fields[0] == "scene")
                {
                    if (fields.size() != 2 || sceneName)
                    {
                        AddDiagnostic(result.diagnostics,
                            sceneName ? SceneSerializationDiagnosticCode::DuplicateSceneRecord :
                                SceneSerializationDiagnosticCode::MalformedRecord,
                            lineNumber, {}, "invalid or duplicate scene record");
                    }
                    else
                    {
                        std::string decoded;
                        if (!DecodeString(fields[1], decoded))
                        {
                            AddDiagnostic(result.diagnostics,
                                SceneSerializationDiagnosticCode::InvalidNameEncoding,
                                lineNumber, {}, "invalid encoded scene name");
                        }
                        else sceneName = std::move(decoded);
                    }
                }
                else if (fields[0] == "entity")
                {
                    if (fields.size() != 15)
                    {
                        AddDiagnostic(result.diagnostics,
                            SceneSerializationDiagnosticCode::MalformedRecord,
                            lineNumber, {}, "entity record must contain 15 fields");
                    }
                    else
                    {
                        ParsedEntity entity;
                        entity.sourceLine = lineNumber;
                        bool valid = EntityId::TryParse(fields[1], entity.id);
                        if (fields[2] != kNone)
                        {
                            valid = EntityId::TryParse(fields[2], entity.parent) && valid;
                        }
                        valid = DecodeString(fields[3], entity.name) && valid;
                        valid = ParseBoolean(fields[4], entity.visible) && valid;
                        valid = ParseFloat(fields[5], entity.transform.position.x) && valid;
                        valid = ParseFloat(fields[6], entity.transform.position.y) && valid;
                        valid = ParseFloat(fields[7], entity.transform.position.z) && valid;
                        valid = ParseFloat(fields[8], entity.transform.rotation.x) && valid;
                        valid = ParseFloat(fields[9], entity.transform.rotation.y) && valid;
                        valid = ParseFloat(fields[10], entity.transform.rotation.z) && valid;
                        valid = ParseFloat(fields[11], entity.transform.rotation.w) && valid;
                        valid = ParseFloat(fields[12], entity.transform.scale.x) && valid;
                        valid = ParseFloat(fields[13], entity.transform.scale.y) && valid;
                        valid = ParseFloat(fields[14], entity.transform.scale.z) && valid;
                        if (!valid || Math::IsZero(entity.transform.rotation.LengthSquared()))
                        {
                            AddDiagnostic(result.diagnostics,
                                SceneSerializationDiagnosticCode::InvalidEntityId,
                                lineNumber, std::string(fields[1]), "invalid entity record");
                        }
                        else if (indices.find(entity.id) != indices.end())
                        {
                            AddDiagnostic(result.diagnostics,
                                SceneSerializationDiagnosticCode::DuplicateEntityId,
                                lineNumber, entity.id.ToString(), "duplicate entity ID");
                        }
                        else
                        {
                            entity.transform.rotation = entity.transform.rotation.Normalized();
                            indices[entity.id] = parsed.size();
                            parsed.push_back(std::move(entity));
                        }
                    }
                }
                else if (fields[0] == "mesh_renderer")
                {
                    if (fields.size() != 14)
                    {
                        AddDiagnostic(result.diagnostics,
                            SceneSerializationDiagnosticCode::MalformedRecord,
                            lineNumber, {}, "mesh_renderer record must contain 14 fields");
                    }
                    else
                    {
                        EntityId id;
                        const auto found = EntityId::TryParse(fields[1], id)
                            ? indices.find(id) : indices.end();
                        if (found == indices.end())
                        {
                            AddDiagnostic(result.diagnostics,
                                SceneSerializationDiagnosticCode::InvalidEntityId,
                                lineNumber, std::string(fields[1]), "mesh renderer references an unknown entity");
                        }
                        else if (parsed[found->second].meshRenderer)
                        {
                            AddDiagnostic(result.diagnostics,
                                SceneSerializationDiagnosticCode::DuplicateComponent,
                                lineNumber, id.ToString(), "duplicate mesh renderer component");
                        }
                        else
                        {
                            MeshRendererComponent component;
                            bool valid = RestoreMesh(fields[2], manifest, registry, component.mesh,
                                result, lineNumber, fields[1]);
                            valid = RestoreMaterial(fields[3], manifest, registry, component.material,
                                result, lineNumber, fields[1]) && valid;
                            valid = ParseBoolean(fields[4], component.visible) && valid;
                            valid = ParseBoolean(fields[5], component.castShadows) && valid;
                            valid = ParseBoolean(fields[6], component.receiveShadows) && valid;
                            if (fields[7] == "manual") component.boundsMode = RenderBoundsMode::Manual;
                            else if (fields[7] == "auto") component.boundsMode = RenderBoundsMode::Automatic;
                            else valid = false;
                            valid = ParseFloat(fields[8], component.localBoundsMin.x) && valid;
                            valid = ParseFloat(fields[9], component.localBoundsMin.y) && valid;
                            valid = ParseFloat(fields[10], component.localBoundsMin.z) && valid;
                            valid = ParseFloat(fields[11], component.localBoundsMax.x) && valid;
                            valid = ParseFloat(fields[12], component.localBoundsMax.y) && valid;
                            valid = ParseFloat(fields[13], component.localBoundsMax.z) && valid;
                            if (!valid)
                            {
                                AddDiagnostic(result.diagnostics,
                                    SceneSerializationDiagnosticCode::MalformedRecord,
                                    lineNumber, id.ToString(), "invalid mesh renderer component");
                            }
                            else parsed[found->second].meshRenderer = component;
                        }
                    }
                }
                else if (fields[0] == "light")
                {
                    if (fields.size() != 17)
                    {
                        AddDiagnostic(result.diagnostics,
                            SceneSerializationDiagnosticCode::MalformedRecord,
                            lineNumber, {}, "light record must contain 17 fields");
                    }
                    else
                    {
                        EntityId id;
                        const auto found = EntityId::TryParse(fields[1], id)
                            ? indices.find(id) : indices.end();
                        if (found == indices.end())
                        {
                            AddDiagnostic(result.diagnostics,
                                SceneSerializationDiagnosticCode::InvalidEntityId,
                                lineNumber, std::string(fields[1]), "light references an unknown entity");
                        }
                        else if (parsed[found->second].light)
                        {
                            AddDiagnostic(result.diagnostics,
                                SceneSerializationDiagnosticCode::DuplicateComponent,
                                lineNumber, id.ToString(), "duplicate light component");
                        }
                        else
                        {
                            LightComponent component;
                            u32 type = 0;
                            bool valid = ParseUnsigned(fields[2], type) &&
                                type <= static_cast<u32>(LightType::Area);
                            component.type = static_cast<LightType>(type);
                            valid = ParseFloat(fields[3], component.localDirection.x) && valid;
                            valid = ParseFloat(fields[4], component.localDirection.y) && valid;
                            valid = ParseFloat(fields[5], component.localDirection.z) && valid;
                            valid = ParseFloat(fields[6], component.color.x) && valid;
                            valid = ParseFloat(fields[7], component.color.y) && valid;
                            valid = ParseFloat(fields[8], component.color.z) && valid;
                            valid = ParseFloat(fields[9], component.intensity) && valid;
                            valid = ParseFloat(fields[10], component.range) && valid;
                            valid = ParseFloat(fields[11], component.innerConeAngle) && valid;
                            valid = ParseFloat(fields[12], component.outerConeAngle) && valid;
                            valid = ParseBoolean(fields[13], component.castShadows) && valid;
                            valid = ParseFloat(fields[14], component.shadowBias) && valid;
                            valid = ParseUnsigned(fields[15], component.shadowMapSize) && valid;
                            valid = ParseBoolean(fields[16], component.enabled) && valid;
                            if (!valid)
                            {
                                AddDiagnostic(result.diagnostics,
                                    SceneSerializationDiagnosticCode::InvalidLightType,
                                    lineNumber, id.ToString(), "invalid light component");
                            }
                            else parsed[found->second].light = component;
                        }
                    }
                }
                else if (fields[0] == "primary_light")
                {
                    if (fields.size() != 2 || primarySeen)
                    {
                        AddDiagnostic(result.diagnostics,
                            SceneSerializationDiagnosticCode::MalformedRecord,
                            lineNumber, {}, "invalid or duplicate primary_light record");
                    }
                    else
                    {
                        primarySeen = true;
                        if (fields[1] != kNone && !EntityId::TryParse(fields[1], primaryLight))
                        {
                            AddDiagnostic(result.diagnostics,
                                SceneSerializationDiagnosticCode::InvalidEntityId,
                                lineNumber, std::string(fields[1]), "invalid primary-light entity ID");
                        }
                    }
                }
                else
                {
                    AddDiagnostic(result.diagnostics,
                        SceneSerializationDiagnosticCode::MalformedRecord,
                        lineNumber, {}, "unknown scene record type");
                }
            }
            if (lineEnd == std::string_view::npos) break;
            lineStart = lineEnd + 1;
        }

        if (!headerSeen)
        {
            AddDiagnostic(result.diagnostics,
                SceneSerializationDiagnosticCode::InvalidHeader, 0, {}, "missing scene header");
        }
        if (!sceneName)
        {
            AddDiagnostic(result.diagnostics,
                SceneSerializationDiagnosticCode::MissingSceneRecord, 0, {}, "missing scene record");
        }

        for (const ParsedEntity& entity : parsed)
        {
            if (entity.parent && indices.find(entity.parent) == indices.end())
            {
                AddDiagnostic(result.diagnostics,
                    SceneSerializationDiagnosticCode::InvalidParent,
                    entity.sourceLine, entity.id.ToString(), "parent entity does not exist");
            }
            std::unordered_set<EntityId, EntityIdHash> visited;
            EntityId current = entity.id;
            while (current)
            {
                if (!visited.insert(current).second)
                {
                    AddDiagnostic(result.diagnostics,
                        SceneSerializationDiagnosticCode::HierarchyCycle,
                        entity.sourceLine, entity.id.ToString(), "entity hierarchy contains a cycle");
                    break;
                }
                const auto iterator = indices.find(current);
                current = iterator == indices.end() ? EntityId{} : parsed[iterator->second].parent;
            }
        }
        if (primaryLight)
        {
            const auto iterator = indices.find(primaryLight);
            if (iterator == indices.end() || !parsed[iterator->second].light)
            {
                AddDiagnostic(result.diagnostics,
                    SceneSerializationDiagnosticCode::InvalidParent,
                    0, primaryLight.ToString(), "primary light does not reference a light entity");
            }
        }
        if (!result.diagnostics.empty())
        {
            return result;
        }

        auto scene = std::make_shared<Scene>(*sceneName);
        for (const ParsedEntity& parsedEntity : parsed)
        {
            Entity entity = scene->CreateEntityWithId(parsedEntity.id, parsedEntity.name);
            entity.SetVisible(parsedEntity.visible);
            entity.SetLocalTransform(
                parsedEntity.transform.position,
                parsedEntity.transform.rotation,
                parsedEntity.transform.scale);
        }
        for (const ParsedEntity& parsedEntity : parsed)
        {
            Entity entity = scene->FindEntity(parsedEntity.id);
            if (parsedEntity.parent && !entity.SetParent(scene->FindEntity(parsedEntity.parent)))
            {
                AddDiagnostic(result.diagnostics,
                    SceneSerializationDiagnosticCode::InvalidParent,
                    parsedEntity.sourceLine, parsedEntity.id.ToString(), "failed to restore parent");
                return result;
            }
            if (parsedEntity.meshRenderer &&
                !entity.SetMeshRenderer(*parsedEntity.meshRenderer, &registry))
            {
                AddDiagnostic(result.diagnostics,
                    SceneSerializationDiagnosticCode::UnresolvedResource,
                    parsedEntity.sourceLine, parsedEntity.id.ToString(), "failed to restore mesh renderer");
                return result;
            }
            if (parsedEntity.light && !entity.SetLight(*parsedEntity.light))
            {
                AddDiagnostic(result.diagnostics,
                    SceneSerializationDiagnosticCode::MalformedRecord,
                    parsedEntity.sourceLine, parsedEntity.id.ToString(), "failed to restore light");
                return result;
            }
            ++result.restoredEntities;
            if (parsedEntity.meshRenderer) ++result.restoredObjects;
            if (parsedEntity.light) ++result.restoredLights;
        }
        if (primaryLight)
        {
            scene->SetPrimaryLight(scene->FindEntity(primaryLight));
        }
        result.scene = std::move(scene);
        return result;
    }
}
