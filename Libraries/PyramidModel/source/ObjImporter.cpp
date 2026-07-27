#include <Pyramid/Model/ObjImporter.hpp>

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace Pyramid::Model
{
    namespace
    {
        constexpr f32 kNormalEpsilonSquared = 1.0e-12f;

        std::string_view Trim(std::string_view value)
        {
            while (!value.empty() &&
                   (value.front() == ' ' || value.front() == '\t' || value.front() == '\r'))
            {
                value.remove_prefix(1);
            }
            while (!value.empty() &&
                   (value.back() == ' ' || value.back() == '\t' || value.back() == '\r'))
            {
                value.remove_suffix(1);
            }
            return value;
        }

        std::string_view RemoveComment(std::string_view value)
        {
            bool quoted = false;
            char quote = '\0';
            for (std::size_t index = 0; index < value.size(); ++index)
            {
                const char character = value[index];
                if ((character == '"' || character == '\'') &&
                    (index == 0 || value[index - 1] != '\\'))
                {
                    if (!quoted)
                    {
                        quoted = true;
                        quote = character;
                    }
                    else if (character == quote)
                    {
                        quoted = false;
                    }
                }
                else if (character == '#' && !quoted)
                {
                    return value.substr(0, index);
                }
            }
            return value;
        }

        std::vector<std::string> Tokenize(std::string_view value)
        {
            std::vector<std::string> tokens;
            std::string token;
            bool quoted = false;
            char quote = '\0';

            for (std::size_t index = 0; index < value.size(); ++index)
            {
                const char character = value[index];
                if (quoted)
                {
                    if (character == quote)
                    {
                        quoted = false;
                    }
                    else if (character == '\\' && index + 1 < value.size() &&
                             value[index + 1] == quote)
                    {
                        token.push_back(value[++index]);
                    }
                    else
                    {
                        token.push_back(character);
                    }
                    continue;
                }

                if (character == '"' || character == '\'')
                {
                    quoted = true;
                    quote = character;
                }
                else if (character == ' ' || character == '\t' || character == '\r')
                {
                    if (!token.empty())
                    {
                        tokens.push_back(std::move(token));
                        token.clear();
                    }
                }
                else
                {
                    token.push_back(character);
                }
            }

            if (!token.empty())
            {
                tokens.push_back(std::move(token));
            }
            return tokens;
        }

        std::string JoinTokens(
            const std::vector<std::string>& tokens,
            std::size_t first)
        {
            std::string result;
            for (std::size_t index = first; index < tokens.size(); ++index)
            {
                if (!result.empty())
                {
                    result.push_back(' ');
                }
                result += tokens[index];
            }
            return result;
        }

        std::string NormalizeSeparators(std::string value)
        {
            std::replace(value.begin(), value.end(), '\\', '/');
            return value;
        }

        std::string NormalizePath(const std::string& value)
        {
            if (value.empty())
            {
                return {};
            }
            return std::filesystem::path(NormalizeSeparators(value))
                .lexically_normal()
                .generic_string();
        }

        std::string ResolvePath(
            const std::string& sourcePath,
            const std::string& referencedPath)
        {
            const std::filesystem::path reference(
                NormalizeSeparators(referencedPath));
            if (reference.is_absolute())
            {
                return reference.lexically_normal().generic_string();
            }

            const std::filesystem::path source(NormalizeSeparators(sourcePath));
            return (source.parent_path() / reference)
                .lexically_normal()
                .generic_string();
        }

        bool ParseFloat(const std::string& token, f32& value)
        {
            errno = 0;
            char* end = nullptr;
            const float parsed = std::strtof(token.c_str(), &end);
            if (end == token.c_str() || !end || *end != '\0' || errno == ERANGE ||
                !std::isfinite(parsed))
            {
                return false;
            }
            value = parsed;
            return true;
        }

        bool ParseInteger(const std::string& token, i32& value)
        {
            errno = 0;
            char* end = nullptr;
            const long parsed = std::strtol(token.c_str(), &end, 10);
            if (end == token.c_str() || !end || *end != '\0' || errno == ERANGE ||
                parsed < std::numeric_limits<i32>::min() ||
                parsed > std::numeric_limits<i32>::max())
            {
                return false;
            }
            value = static_cast<i32>(parsed);
            return true;
        }

        template <typename Callback>
        void ForEachLine(std::string_view text, Callback&& callback)
        {
            u32 lineNumber = 1;
            std::size_t start = 0;
            while (start <= text.size())
            {
                const std::size_t end = text.find('\n', start);
                const std::size_t length = end == std::string_view::npos
                    ? text.size() - start
                    : end - start;
                callback(text.substr(start, length), lineNumber);
                if (end == std::string_view::npos)
                {
                    break;
                }
                start = end + 1;
                ++lineNumber;
            }
        }

        class DiagnosticSink
        {
        public:
            DiagnosticSink(ImportedModel& model, u32 maximumDiagnostics)
                : m_model(model),
                  m_maximumDiagnostics(std::max<u32>(1, maximumDiagnostics))
            {
            }

            void Error(const std::string& source, u32 line, std::string message)
            {
                Add(ImportDiagnosticSeverity::Error, source, line, std::move(message));
            }

            void Warning(const std::string& source, u32 line, std::string message)
            {
                Add(ImportDiagnosticSeverity::Warning, source, line, std::move(message));
            }

        private:
            void Add(
                ImportDiagnosticSeverity severity,
                const std::string& source,
                u32 line,
                std::string message)
            {
                if (m_model.diagnostics.size() >= m_maximumDiagnostics)
                {
                    return;
                }
                m_model.diagnostics.push_back({severity, line, source, std::move(message)});
            }

            ImportedModel& m_model;
            u32 m_maximumDiagnostics = 0;
        };

        bool ReadTextFile(
            const std::string& filepath,
            std::size_t maximumBytes,
            std::string& text,
            std::string& error)
        {
            std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
            if (!stream)
            {
                error = "could not open file";
                return false;
            }

            const std::streamoff end = stream.tellg();
            if (end < 0)
            {
                error = "could not determine file size";
                return false;
            }
            if (static_cast<unsigned long long>(end) > maximumBytes)
            {
                error = "file exceeds configured size limit";
                return false;
            }

            text.resize(static_cast<std::size_t>(end));
            stream.seekg(0, std::ios::beg);
            if (!text.empty() &&
                !stream.read(text.data(), static_cast<std::streamsize>(text.size())))
            {
                error = "could not read complete file";
                text.clear();
                return false;
            }
            return true;
        }

        struct MaterialReference
        {
            std::string path;
            u32 line = 0;
        };

        std::vector<MaterialReference> FindMaterialReferences(
            std::string_view objText,
            const std::string& sourcePath)
        {
            std::vector<MaterialReference> references;
            ForEachLine(objText, [&](std::string_view rawLine, u32 lineNumber)
            {
                const auto tokens = Tokenize(Trim(RemoveComment(rawLine)));
                if (tokens.size() < 2 || tokens.front() != "mtllib")
                {
                    return;
                }
                for (std::size_t index = 1; index < tokens.size(); ++index)
                {
                    references.push_back({ResolvePath(sourcePath, tokens[index]), lineNumber});
                }
            });
            return references;
        }

        std::string ParseTexturePath(const std::vector<std::string>& tokens)
        {
            std::size_t index = 1;
            while (index < tokens.size() && !tokens[index].empty() && tokens[index][0] == '-')
            {
                const std::string& option = tokens[index++];
                if (option == "-o" || option == "-s" || option == "-t")
                {
                    std::size_t arguments = 0;
                    while (index + arguments < tokens.size() && arguments < 3)
                    {
                        f32 ignoredValue = 0.0f;
                        if (!ParseFloat(tokens[index + arguments], ignoredValue))
                        {
                            break;
                        }
                        ++arguments;
                    }
                    index += arguments;
                }
                else if (option == "-mm")
                {
                    std::size_t arguments = 0;
                    while (index + arguments < tokens.size() && arguments < 2)
                    {
                        f32 ignoredValue = 0.0f;
                        if (!ParseFloat(tokens[index + arguments], ignoredValue))
                        {
                            break;
                        }
                        ++arguments;
                    }
                    index += arguments;
                }
                else if (index < tokens.size())
                {
                    ++index;
                }
            }
            return JoinTokens(tokens, index);
        }

        bool ParseVec3(
            const std::vector<std::string>& tokens,
            Math::Vec3& value)
        {
            return tokens.size() >= 4 &&
                ParseFloat(tokens[1], value.x) &&
                ParseFloat(tokens[2], value.y) &&
                ParseFloat(tokens[3], value.z);
        }

        void ParseMaterialLibrary(
            const ObjMaterialLibrarySource& source,
            ImportedModel& model,
            std::unordered_map<std::string, i32>& materialIndices,
            DiagnosticSink& diagnostics,
            const ObjImportOptions& options)
        {
            if (source.text.size() > options.limits.maximumMaterialBytes)
            {
                diagnostics.Error(source.sourcePath, 0, "material library exceeds configured size limit");
                return;
            }

            i32 currentMaterial = -1;
            ForEachLine(source.text, [&](std::string_view rawLine, u32 lineNumber)
            {
                const auto tokens = Tokenize(Trim(RemoveComment(rawLine)));
                if (tokens.empty())
                {
                    return;
                }

                const std::string& command = tokens.front();
                if (command == "newmtl")
                {
                    const std::string name = JoinTokens(tokens, 1);
                    if (name.empty())
                    {
                        diagnostics.Error(source.sourcePath, lineNumber, "newmtl requires a material name");
                        currentMaterial = -1;
                        return;
                    }
                    if (materialIndices.find(name) != materialIndices.end())
                    {
                        diagnostics.Error(source.sourcePath, lineNumber, "duplicate material name: " + name);
                        currentMaterial = -1;
                        return;
                    }
                    if (model.materials.size() >= options.limits.maximumMaterials)
                    {
                        diagnostics.Error(source.sourcePath, lineNumber, "material count exceeds configured limit");
                        currentMaterial = -1;
                        return;
                    }

                    currentMaterial = static_cast<i32>(model.materials.size());
                    ImportedMaterial material;
                    material.name = name;
                    model.materials.push_back(std::move(material));
                    materialIndices.emplace(name, currentMaterial);
                    return;
                }

                if (currentMaterial < 0)
                {
                    diagnostics.Warning(
                        source.sourcePath,
                        lineNumber,
                        "material property appears before newmtl and was ignored");
                    return;
                }

                ImportedMaterial& material = model.materials[static_cast<std::size_t>(currentMaterial)];
                if (command == "Ka")
                {
                    if (!ParseVec3(tokens, material.ambientColor))
                    {
                        diagnostics.Error(source.sourcePath, lineNumber, "Ka requires three finite values");
                    }
                }
                else if (command == "Kd")
                {
                    if (!ParseVec3(tokens, material.diffuseColor))
                    {
                        diagnostics.Error(source.sourcePath, lineNumber, "Kd requires three finite values");
                    }
                }
                else if (command == "Ks")
                {
                    if (!ParseVec3(tokens, material.specularColor))
                    {
                        diagnostics.Error(source.sourcePath, lineNumber, "Ks requires three finite values");
                    }
                }
                else if (command == "Ns")
                {
                    if (tokens.size() < 2 || !ParseFloat(tokens[1], material.specularExponent))
                    {
                        diagnostics.Error(source.sourcePath, lineNumber, "Ns requires one finite value");
                    }
                }
                else if (command == "d")
                {
                    const std::size_t valueIndex = tokens.size() > 2 && tokens[1] == "-halo" ? 2u : 1u;
                    if (tokens.size() <= valueIndex ||
                        !ParseFloat(tokens[valueIndex], material.opacity))
                    {
                        diagnostics.Error(source.sourcePath, lineNumber, "d requires one finite value");
                    }
                    else
                    {
                        material.opacity = Math::Clamp(material.opacity, 0.0f, 1.0f);
                    }
                }
                else if (command == "Tr")
                {
                    f32 transparency = 0.0f;
                    if (tokens.size() < 2 || !ParseFloat(tokens[1], transparency))
                    {
                        diagnostics.Error(source.sourcePath, lineNumber, "Tr requires one finite value");
                    }
                    else
                    {
                        material.opacity = 1.0f - Math::Clamp(transparency, 0.0f, 1.0f);
                    }
                }
                else if (command == "illum")
                {
                    if (tokens.size() < 2 || !ParseInteger(tokens[1], material.illuminationModel))
                    {
                        diagnostics.Error(source.sourcePath, lineNumber, "illum requires one integer value");
                    }
                }
                else if (command == "map_Kd")
                {
                    const std::string texturePath = ParseTexturePath(tokens);
                    if (texturePath.empty())
                    {
                        diagnostics.Error(source.sourcePath, lineNumber, "map_Kd requires a texture path");
                    }
                    else
                    {
                        material.diffuseTexture = ResolvePath(source.sourcePath, texturePath);
                    }
                }
            });
        }

        struct FaceVertexReference
        {
            i32 position = -1;
            i32 texCoord = -1;
            i32 normal = -1;
        };

        bool ResolveIndex(i32 rawIndex, std::size_t count, i32& resolved)
        {
            if (rawIndex == 0)
            {
                return false;
            }
            const long long candidate = rawIndex > 0
                ? static_cast<long long>(rawIndex) - 1
                : static_cast<long long>(count) + rawIndex;
            if (candidate < 0 || candidate >= static_cast<long long>(count))
            {
                return false;
            }
            resolved = static_cast<i32>(candidate);
            return true;
        }

        bool ParseFaceReference(
            const std::string& token,
            std::size_t positionCount,
            std::size_t texCoordCount,
            std::size_t normalCount,
            FaceVertexReference& reference,
            std::string& error)
        {
            std::string parts[3];
            std::size_t part = 0;
            for (char character : token)
            {
                if (character == '/')
                {
                    if (part == 2)
                    {
                        error = "face vertex contains too many separators";
                        return false;
                    }
                    ++part;
                }
                else
                {
                    parts[part].push_back(character);
                }
            }

            i32 raw = 0;
            if (parts[0].empty() || !ParseInteger(parts[0], raw) ||
                !ResolveIndex(raw, positionCount, reference.position))
            {
                error = "face position index is invalid or out of range";
                return false;
            }

            if (part >= 1 && !parts[1].empty())
            {
                if (!ParseInteger(parts[1], raw) ||
                    !ResolveIndex(raw, texCoordCount, reference.texCoord))
                {
                    error = "face texture-coordinate index is invalid or out of range";
                    return false;
                }
            }

            if (part >= 2 && !parts[2].empty())
            {
                if (!ParseInteger(parts[2], raw) ||
                    !ResolveIndex(raw, normalCount, reference.normal))
                {
                    error = "face normal index is invalid or out of range";
                    return false;
                }
            }
            return true;
        }

        struct VertexKey
        {
            i32 position = -1;
            i32 texCoord = -1;
            i32 normal = -1;
            i64 generatedNormalGroup = 0;

            bool operator==(const VertexKey& other) const
            {
                return position == other.position && texCoord == other.texCoord &&
                    normal == other.normal &&
                    generatedNormalGroup == other.generatedNormalGroup;
            }
        };

        struct VertexKeyHash
        {
            std::size_t operator()(const VertexKey& value) const noexcept
            {
                std::size_t hash = static_cast<std::size_t>(value.position + 1);
                hash = hash * 16777619u ^ static_cast<std::size_t>(value.texCoord + 2);
                hash = hash * 16777619u ^ static_cast<std::size_t>(value.normal + 2);
                hash = hash * 16777619u ^
                    static_cast<std::size_t>(value.generatedNormalGroup);
                return hash;
            }
        };

        struct PrimitiveBuilder
        {
            std::string name;
            i32 materialIndex = -1;
            std::vector<ModelVertex> vertices;
            std::vector<u32> indices;
            std::vector<Math::Vec3> generatedNormalSums;
            std::vector<bool> generatedNormals;
            std::unordered_map<VertexKey, u32, VertexKeyHash> vertexIndices;
            bool allVerticesHaveTexCoords = true;
        };

        std::string ComposePrimitiveName(
            const std::string& objectName,
            const std::string& groupName,
            const std::string& materialName)
        {
            std::string name = objectName.empty() ? "Object" : objectName;
            if (!groupName.empty() && groupName != objectName)
            {
                name += "/" + groupName;
            }
            if (!materialName.empty())
            {
                name += ":" + materialName;
            }
            return name;
        }

        bool IsFinite(const Math::Vec3& value)
        {
            return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
        }

        class ObjGeometryParser
        {
        public:
            ObjGeometryParser(
                const ObjImportRequest& request,
                const ObjImportOptions& options,
                ImportedModel& model,
                const std::unordered_map<std::string, i32>& materialIndices,
                DiagnosticSink& diagnostics)
                : m_request(request),
                  m_options(options),
                  m_model(model),
                  m_materialIndices(materialIndices),
                  m_diagnostics(diagnostics)
            {
            }

            void Parse()
            {
                ForEachLine(m_request.objText, [&](std::string_view rawLine, u32 lineNumber)
                {
                    ParseLine(rawLine, lineNumber);
                });
                FlushPrimitive();

                if (m_model.primitives.empty() && !m_model.HasErrors())
                {
                    m_diagnostics.Error(m_request.sourcePath, 0, "OBJ contains no renderable faces");
                }
            }

        private:
            void ParseLine(std::string_view rawLine, u32 lineNumber)
            {
                const auto tokens = Tokenize(Trim(RemoveComment(rawLine)));
                if (tokens.empty())
                {
                    return;
                }

                const std::string& command = tokens.front();
                if (command == "v")
                {
                    ParsePosition(tokens, lineNumber);
                }
                else if (command == "vt")
                {
                    ParseTexCoord(tokens, lineNumber);
                }
                else if (command == "vn")
                {
                    ParseNormal(tokens, lineNumber);
                }
                else if (command == "f")
                {
                    ParseFace(tokens, lineNumber);
                }
                else if (command == "o")
                {
                    FlushPrimitive();
                    m_objectName = JoinTokens(tokens, 1);
                }
                else if (command == "g")
                {
                    FlushPrimitive();
                    m_groupName = JoinTokens(tokens, 1);
                }
                else if (command == "usemtl")
                {
                    FlushPrimitive();
                    m_materialName = JoinTokens(tokens, 1);
                    const auto found = m_materialIndices.find(m_materialName);
                    if (found == m_materialIndices.end())
                    {
                        m_materialIndex = -1;
                        m_diagnostics.Warning(
                            m_request.sourcePath,
                            lineNumber,
                            "unknown material: " + m_materialName);
                    }
                    else
                    {
                        m_materialIndex = found->second;
                    }
                }
                else if (command == "s")
                {
                    ParseSmoothing(tokens);
                }
            }

            void ParsePosition(const std::vector<std::string>& tokens, u32 lineNumber)
            {
                if (m_positions.size() >= m_options.limits.maximumPositions)
                {
                    m_diagnostics.Error(m_request.sourcePath, lineNumber, "position count exceeds configured limit");
                    return;
                }

                Math::Vec3 position;
                if (tokens.size() < 4 || !ParseFloat(tokens[1], position.x) ||
                    !ParseFloat(tokens[2], position.y) || !ParseFloat(tokens[3], position.z))
                {
                    m_diagnostics.Error(m_request.sourcePath, lineNumber, "v requires three finite position values");
                    return;
                }

                Math::Vec4 color(1.0f);
                if (tokens.size() == 7 || tokens.size() == 8)
                {
                    if (!ParseFloat(tokens[4], color.x) || !ParseFloat(tokens[5], color.y) ||
                        !ParseFloat(tokens[6], color.z) ||
                        (tokens.size() == 8 && !ParseFloat(tokens[7], color.w)))
                    {
                        m_diagnostics.Error(m_request.sourcePath, lineNumber, "vertex color contains a non-finite value");
                        return;
                    }
                }

                m_positions.push_back(position);
                m_positionColors.push_back(color);
            }

            void ParseTexCoord(const std::vector<std::string>& tokens, u32 lineNumber)
            {
                if (m_texCoords.size() >= m_options.limits.maximumTexCoords)
                {
                    m_diagnostics.Error(m_request.sourcePath, lineNumber, "texture-coordinate count exceeds configured limit");
                    return;
                }

                Math::Vec2 texCoord;
                if (tokens.size() < 3 || !ParseFloat(tokens[1], texCoord.x) ||
                    !ParseFloat(tokens[2], texCoord.y))
                {
                    m_diagnostics.Error(m_request.sourcePath, lineNumber, "vt requires two finite values");
                    return;
                }
                if (m_options.flipTexCoordV)
                {
                    texCoord.y = 1.0f - texCoord.y;
                }
                m_texCoords.push_back(texCoord);
            }

            void ParseNormal(const std::vector<std::string>& tokens, u32 lineNumber)
            {
                if (m_normals.size() >= m_options.limits.maximumNormals)
                {
                    m_diagnostics.Error(m_request.sourcePath, lineNumber, "normal count exceeds configured limit");
                    return;
                }

                Math::Vec3 normal;
                if (tokens.size() < 4 || !ParseFloat(tokens[1], normal.x) ||
                    !ParseFloat(tokens[2], normal.y) || !ParseFloat(tokens[3], normal.z))
                {
                    m_diagnostics.Error(m_request.sourcePath, lineNumber, "vn requires three finite values");
                    return;
                }
                if (normal.LengthSquared() > kNormalEpsilonSquared)
                {
                    normal.Normalize();
                }
                m_normals.push_back(normal);
            }

            void ParseSmoothing(const std::vector<std::string>& tokens)
            {
                if (tokens.size() < 2 || tokens[1] == "off" || tokens[1] == "0")
                {
                    m_smoothingGroup = 0;
                    return;
                }

                const auto found = m_smoothingGroups.find(tokens[1]);
                if (found != m_smoothingGroups.end())
                {
                    m_smoothingGroup = found->second;
                    return;
                }
                m_smoothingGroup = m_nextSmoothingGroup++;
                m_smoothingGroups.emplace(tokens[1], m_smoothingGroup);
            }

            void ParseFace(const std::vector<std::string>& tokens, u32 lineNumber)
            {
                if (tokens.size() < 4)
                {
                    m_diagnostics.Error(m_request.sourcePath, lineNumber, "f requires at least three vertices");
                    return;
                }
                const std::size_t cornerCount = tokens.size() - 1;
                if (cornerCount > m_options.limits.maximumFaceVertices)
                {
                    m_diagnostics.Error(m_request.sourcePath, lineNumber, "face vertex count exceeds configured limit");
                    return;
                }

                std::vector<FaceVertexReference> corners(cornerCount);
                for (std::size_t index = 0; index < cornerCount; ++index)
                {
                    std::string error;
                    if (!ParseFaceReference(
                            tokens[index + 1],
                            m_positions.size(),
                            m_texCoords.size(),
                            m_normals.size(),
                            corners[index],
                            error))
                    {
                        m_diagnostics.Error(m_request.sourcePath, lineNumber, std::move(error));
                        return;
                    }
                }

                ++m_faceSerial;
                for (std::size_t index = 1; index + 1 < corners.size(); ++index)
                {
                    if (!AppendTriangle(corners[0], corners[index], corners[index + 1], lineNumber))
                    {
                        return;
                    }
                }
            }

            bool AppendTriangle(
                const FaceVertexReference& first,
                const FaceVertexReference& second,
                const FaceVertexReference& third,
                u32 lineNumber)
            {
                EnsurePrimitive();
                if (m_totalIndices + m_builder.indices.size() + 3u >
                    m_options.limits.maximumIndices)
                {
                    m_diagnostics.Error(m_request.sourcePath, lineNumber, "index count exceeds configured limit");
                    return false;
                }

                const Math::Vec3 edgeA = m_positions[static_cast<std::size_t>(second.position)] -
                    m_positions[static_cast<std::size_t>(first.position)];
                const Math::Vec3 edgeB = m_positions[static_cast<std::size_t>(third.position)] -
                    m_positions[static_cast<std::size_t>(first.position)];
                const Math::Vec3 faceNormal = edgeA.Cross(edgeB);
                if (!IsFinite(faceNormal) || faceNormal.LengthSquared() <= kNormalEpsilonSquared)
                {
                    m_diagnostics.Warning(m_request.sourcePath, lineNumber, "degenerate triangle has no usable generated normal");
                }

                const FaceVertexReference references[3] = {first, second, third};
                for (const auto& reference : references)
                {
                    const u32 vertexIndex = GetOrCreateVertex(reference, faceNormal, lineNumber);
                    if (vertexIndex == std::numeric_limits<u32>::max())
                    {
                        return false;
                    }
                    m_builder.indices.push_back(vertexIndex);
                }
                return true;
            }

            u32 GetOrCreateVertex(
                const FaceVertexReference& reference,
                const Math::Vec3& faceNormal,
                u32 lineNumber)
            {
                i64 generatedGroup = 0;
                if (reference.normal < 0 && m_options.generateMissingNormals)
                {
                    generatedGroup = m_smoothingGroup == 0
                        ? -static_cast<i64>(m_faceSerial)
                        : m_smoothingGroup;
                }

                const VertexKey key{
                    reference.position,
                    reference.texCoord,
                    reference.normal,
                    generatedGroup};
                const auto found = m_builder.vertexIndices.find(key);
                if (found != m_builder.vertexIndices.end())
                {
                    if (reference.normal < 0 && m_options.generateMissingNormals)
                    {
                        m_builder.generatedNormalSums[found->second] += faceNormal;
                    }
                    return found->second;
                }

                if (m_totalVertices + m_builder.vertices.size() + 1u >
                    m_options.limits.maximumVertices)
                {
                    m_diagnostics.Error(m_request.sourcePath, lineNumber, "vertex count exceeds configured limit");
                    return std::numeric_limits<u32>::max();
                }

                ModelVertex vertex;
                vertex.position = m_positions[static_cast<std::size_t>(reference.position)];
                vertex.color = m_positionColors[static_cast<std::size_t>(reference.position)];
                if (reference.texCoord >= 0)
                {
                    vertex.texCoord = m_texCoords[static_cast<std::size_t>(reference.texCoord)];
                }
                else
                {
                    m_builder.allVerticesHaveTexCoords = false;
                }
                if (reference.normal >= 0)
                {
                    vertex.normal = m_normals[static_cast<std::size_t>(reference.normal)];
                }

                const bool generated = reference.normal < 0 && m_options.generateMissingNormals;
                const u32 index = static_cast<u32>(m_builder.vertices.size());
                m_builder.vertices.push_back(vertex);
                m_builder.generatedNormals.push_back(generated);
                m_builder.generatedNormalSums.push_back(generated ? faceNormal : Math::Vec3::Zero);
                m_builder.vertexIndices.emplace(key, index);
                return index;
            }

            void EnsurePrimitive()
            {
                if (!m_builder.name.empty())
                {
                    return;
                }
                m_builder.name = ComposePrimitiveName(m_objectName, m_groupName, m_materialName);
                m_builder.materialIndex = m_materialIndex;
            }

            void FlushPrimitive()
            {
                if (m_builder.indices.empty())
                {
                    m_builder = {};
                    return;
                }
                if (m_model.primitives.size() >= m_options.limits.maximumPrimitives)
                {
                    m_diagnostics.Error(m_request.sourcePath, 0, "primitive count exceeds configured limit");
                    m_builder = {};
                    return;
                }

                ImportedPrimitive primitive;
                primitive.name = MakeUniqueName(m_builder.name);
                primitive.materialIndex = m_builder.materialIndex;
                primitive.vertices = std::move(m_builder.vertices);
                primitive.indices = std::move(m_builder.indices);

                bool hasNormals = true;
                const bool hasTexCoords = m_builder.allVerticesHaveTexCoords;
                for (std::size_t index = 0; index < primitive.vertices.size(); ++index)
                {
                    if (m_builder.generatedNormals[index])
                    {
                        Math::Vec3 normal = m_builder.generatedNormalSums[index];
                        if (normal.LengthSquared() > kNormalEpsilonSquared)
                        {
                            normal.Normalize();
                            primitive.vertices[index].normal = normal;
                        }
                    }
                    hasNormals = hasNormals &&
                        primitive.vertices[index].normal.LengthSquared() > kNormalEpsilonSquared;
                }

                primitive.hasNormals = hasNormals;
                primitive.hasTexCoords = hasTexCoords;

                primitive.boundsMin = primitive.vertices.front().position;
                primitive.boundsMax = primitive.vertices.front().position;
                for (const auto& vertex : primitive.vertices)
                {
                    primitive.boundsMin.x = Math::Min(primitive.boundsMin.x, vertex.position.x);
                    primitive.boundsMin.y = Math::Min(primitive.boundsMin.y, vertex.position.y);
                    primitive.boundsMin.z = Math::Min(primitive.boundsMin.z, vertex.position.z);
                    primitive.boundsMax.x = Math::Max(primitive.boundsMax.x, vertex.position.x);
                    primitive.boundsMax.y = Math::Max(primitive.boundsMax.y, vertex.position.y);
                    primitive.boundsMax.z = Math::Max(primitive.boundsMax.z, vertex.position.z);
                }

                m_totalVertices += static_cast<u32>(primitive.vertices.size());
                m_totalIndices += static_cast<u32>(primitive.indices.size());
                m_model.primitives.push_back(std::move(primitive));
                m_builder = {};
            }

            std::string MakeUniqueName(const std::string& requested)
            {
                const u32 count = ++m_nameCounts[requested];
                if (count == 1)
                {
                    return requested;
                }
                return requested + "#" + std::to_string(count);
            }

            const ObjImportRequest& m_request;
            const ObjImportOptions& m_options;
            ImportedModel& m_model;
            const std::unordered_map<std::string, i32>& m_materialIndices;
            DiagnosticSink& m_diagnostics;

            std::vector<Math::Vec3> m_positions;
            std::vector<Math::Vec4> m_positionColors;
            std::vector<Math::Vec2> m_texCoords;
            std::vector<Math::Vec3> m_normals;
            PrimitiveBuilder m_builder;
            std::string m_objectName = "Object";
            std::string m_groupName;
            std::string m_materialName;
            i32 m_materialIndex = -1;
            i64 m_smoothingGroup = 0;
            i64 m_nextSmoothingGroup = 1;
            u64 m_faceSerial = 0;
            u32 m_totalVertices = 0;
            u32 m_totalIndices = 0;
            std::unordered_map<std::string, i64> m_smoothingGroups;
            std::unordered_map<std::string, u32> m_nameCounts;
        };
    }

    ImportedModel ObjImporter::Import(
        const ObjImportRequest& request,
        const ObjImportOptions& options)
    {
        ImportedModel model;
        model.sourcePath = NormalizePath(request.sourcePath);
        DiagnosticSink diagnostics(model, options.limits.maximumDiagnostics);

        if (request.objText.size() > options.limits.maximumObjBytes)
        {
            diagnostics.Error(model.sourcePath, 0, "OBJ source exceeds configured size limit");
            return model;
        }
        if (request.objText.empty())
        {
            diagnostics.Error(model.sourcePath, 0, "OBJ source is empty");
            return model;
        }

        std::unordered_map<std::string, const ObjMaterialLibrarySource*> availableLibraries;
        for (const auto& library : request.materialLibraries)
        {
            availableLibraries[NormalizePath(library.sourcePath)] = &library;
        }

        std::unordered_map<std::string, i32> materialIndices;
        std::unordered_set<std::string> parsedLibraries;
        const auto references = FindMaterialReferences(request.objText, model.sourcePath);
        for (const auto& reference : references)
        {
            if (!parsedLibraries.insert(reference.path).second)
            {
                continue;
            }
            const auto found = availableLibraries.find(reference.path);
            if (found == availableLibraries.end())
            {
                const auto severityMessage = "declared material library was not provided: " + reference.path;
                if (options.requireDeclaredMaterialLibraries)
                {
                    diagnostics.Error(model.sourcePath, reference.line, severityMessage);
                }
                else
                {
                    diagnostics.Warning(model.sourcePath, reference.line, severityMessage);
                }
                continue;
            }
            ParseMaterialLibrary(
                *found->second,
                model,
                materialIndices,
                diagnostics,
                options);
        }

        ObjGeometryParser parser(request, options, model, materialIndices, diagnostics);
        parser.Parse();
        return model;
    }

    ImportedModel ObjImporter::ImportFile(
        const std::string& filepath,
        const ObjImportOptions& options)
    {
        ObjImportRequest request;
        request.sourcePath = NormalizePath(filepath);

        std::string error;
        if (!ReadTextFile(filepath, options.limits.maximumObjBytes, request.objText, error))
        {
            ImportedModel model;
            model.sourcePath = request.sourcePath;
            model.diagnostics.push_back({
                ImportDiagnosticSeverity::Error,
                0,
                request.sourcePath,
                "OBJ " + error});
            return model;
        }

        std::unordered_set<std::string> loaded;
        const auto references = FindMaterialReferences(request.objText, request.sourcePath);
        for (const auto& reference : references)
        {
            if (!loaded.insert(reference.path).second)
            {
                continue;
            }

            ObjMaterialLibrarySource library;
            library.sourcePath = reference.path;
            if (!ReadTextFile(
                    reference.path,
                    options.limits.maximumMaterialBytes,
                    library.text,
                    error))
            {
                continue;
            }
            request.materialLibraries.push_back(std::move(library));
        }

        return Import(request, options);
    }
}
