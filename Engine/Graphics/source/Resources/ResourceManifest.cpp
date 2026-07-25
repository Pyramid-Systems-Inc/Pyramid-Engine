#include <Pyramid/Graphics/Resources/ResourceManifest.hpp>

#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>

#include <algorithm>
#include <charconv>
#include <cctype>
#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>

namespace Pyramid
{
    namespace
    {
        constexpr std::string_view kHeader = "PYRAMID_RESOURCE_MANIFEST";

        bool IsManifestKeyValid(std::string_view key)
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

        std::string_view TypeName(ResourceType type)
        {
            switch (type)
            {
            case ResourceType::Mesh: return "mesh";
            case ResourceType::Shader: return "shader";
            case ResourceType::Texture: return "texture";
            case ResourceType::Material: return "material";
            }
            return {};
        }

        bool ParseType(std::string_view value, ResourceType& type)
        {
            if (value == "mesh") type = ResourceType::Mesh;
            else if (value == "shader") type = ResourceType::Shader;
            else if (value == "texture") type = ResourceType::Texture;
            else if (value == "material") type = ResourceType::Material;
            else return false;
            return true;
        }

        std::vector<std::string_view> SplitTabs(std::string_view line)
        {
            std::vector<std::string_view> fields;
            std::size_t start = 0;
            while (true)
            {
                const std::size_t end = line.find('\t', start);
                fields.push_back(line.substr(start, end == std::string_view::npos ? end : end - start));
                if (end == std::string_view::npos) break;
                start = end + 1;
            }
            return fields;
        }

        bool ParseUnsigned(std::string_view text, u32& value)
        {
            if (text.empty()) return false;
            u64 parsed = 0;
            const auto result = std::from_chars(text.data(), text.data() + text.size(), parsed, 10);
            if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
                parsed > std::numeric_limits<u32>::max())
            {
                return false;
            }
            value = static_cast<u32>(parsed);
            return true;
        }

        bool ParseHex64(std::string_view text, u64& value)
        {
            if (text.size() != 16) return false;
            value = 0;
            for (const char character : text)
            {
                value <<= 4U;
                if (character >= '0' && character <= '9') value |= static_cast<u64>(character - '0');
                else if (character >= 'a' && character <= 'f') value |= static_cast<u64>(character - 'a' + 10);
                else if (character >= 'A' && character <= 'F') value |= static_cast<u64>(character - 'A' + 10);
                else return false;
            }
            return true;
        }

        bool ParseAssetId(std::string_view text, u64& high, u64& low)
        {
            return text.size() == 32 && ParseHex64(text.substr(0, 16), high) &&
                ParseHex64(text.substr(16, 16), low) && (high != 0 || low != 0);
        }

        std::string AssetIdToString(u64 high, u64 low)
        {
            std::ostringstream stream;
            stream << std::hex << std::setfill('0') << std::setw(16) << high
                   << std::setw(16) << low;
            return stream.str();
        }

        void AddDiagnostic(
            std::vector<ResourceManifestDiagnostic>& diagnostics,
            ResourceManifestDiagnosticCode code,
            u32 line,
            std::string key,
            std::string message)
        {
            diagnostics.push_back({code, line, std::move(key), std::move(message)});
        }
    }

    bool ResourceManifestEntry::IsValid() const
    {
        return IsManifestKeyValid(key) && (assetIdHigh != 0 || assetIdLow != 0) && generation != 0;
    }

    bool ResourceManifest::AddEntry(
        std::string key,
        ResourceType type,
        u64 high,
        u64 low,
        u32 generation)
    {
        if (!IsManifestKeyValid(key) || (high == 0 && low == 0) || generation == 0 ||
            Contains(key))
        {
            return false;
        }
        m_entries.push_back({std::move(key), type, high, low, generation, 0});
        return true;
    }

    bool ResourceManifest::Add(std::string key, MeshHandle handle)
    {
        const auto id = handle.GetAssetId();
        return handle && AddEntry(std::move(key), ResourceType::Mesh, id.high, id.low, handle.GetGeneration());
    }

    bool ResourceManifest::Add(std::string key, ShaderHandle handle)
    {
        const auto id = handle.GetAssetId();
        return handle && AddEntry(std::move(key), ResourceType::Shader, id.high, id.low, handle.GetGeneration());
    }

    bool ResourceManifest::Add(std::string key, TextureHandle handle)
    {
        const auto id = handle.GetAssetId();
        return handle && AddEntry(std::move(key), ResourceType::Texture, id.high, id.low, handle.GetGeneration());
    }

    bool ResourceManifest::Add(std::string key, MaterialHandle handle)
    {
        const auto id = handle.GetAssetId();
        return handle && AddEntry(std::move(key), ResourceType::Material, id.high, id.low, handle.GetGeneration());
    }

    bool ResourceManifest::Remove(std::string_view key)
    {
        const auto iterator = std::find_if(m_entries.begin(), m_entries.end(),
            [key](const ResourceManifestEntry& entry) { return entry.key == key; });
        if (iterator == m_entries.end()) return false;
        m_entries.erase(iterator);
        return true;
    }

    bool ResourceManifest::Contains(std::string_view key) const
    {
        return std::any_of(m_entries.begin(), m_entries.end(),
            [key](const ResourceManifestEntry& entry) { return entry.key == key; });
    }

    const ResourceManifestEntry* ResourceManifest::FindEntry(
        std::string_view key,
        ResourceType type) const
    {
        const auto iterator = std::find_if(m_entries.begin(), m_entries.end(),
            [key, type](const ResourceManifestEntry& entry)
            {
                return entry.key == key && entry.type == type;
            });
        return iterator == m_entries.end() ? nullptr : &*iterator;
    }

    MeshHandle ResourceManifest::GetMeshHandle(std::string_view key) const
    {
        const auto* entry = FindEntry(key, ResourceType::Mesh);
        return entry ? MeshHandle::FromParts({entry->assetIdHigh, entry->assetIdLow}, entry->generation) : MeshHandle{};
    }

    ShaderHandle ResourceManifest::GetShaderHandle(std::string_view key) const
    {
        const auto* entry = FindEntry(key, ResourceType::Shader);
        return entry ? ShaderHandle::FromParts({entry->assetIdHigh, entry->assetIdLow}, entry->generation) : ShaderHandle{};
    }

    TextureHandle ResourceManifest::GetTextureHandle(std::string_view key) const
    {
        const auto* entry = FindEntry(key, ResourceType::Texture);
        return entry ? TextureHandle::FromParts({entry->assetIdHigh, entry->assetIdLow}, entry->generation) : TextureHandle{};
    }

    MaterialHandle ResourceManifest::GetMaterialHandle(std::string_view key) const
    {
        const auto* entry = FindEntry(key, ResourceType::Material);
        return entry ? MaterialHandle::FromParts({entry->assetIdHigh, entry->assetIdLow}, entry->generation) : MaterialHandle{};
    }

    std::string ResourceManifest::Serialize() const
    {
        std::vector<ResourceManifestEntry> entries = m_entries;
        std::sort(entries.begin(), entries.end(),
            [](const ResourceManifestEntry& left, const ResourceManifestEntry& right)
            {
                return left.key < right.key;
            });

        std::ostringstream stream;
        stream << kHeader << '\t' << kResourceManifestVersion << '\n';
        for (const auto& entry : entries)
        {
            stream << TypeName(entry.type) << '\t' << entry.key << '\t'
                   << AssetIdToString(entry.assetIdHigh, entry.assetIdLow) << '\t'
                   << entry.generation << '\n';
        }
        return stream.str();
    }

    bool ResourceManifest::Deserialize(
        std::string_view text,
        ResourceManifest& output,
        std::vector<ResourceManifestDiagnostic>& diagnostics)
    {
        diagnostics.clear();
        ResourceManifest parsed;
        std::size_t offset = 0;
        u32 lineNumber = 0;
        bool headerRead = false;

        while (offset <= text.size())
        {
            const std::size_t end = text.find('\n', offset);
            std::string_view line = text.substr(offset, end == std::string_view::npos ? end : end - offset);
            if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
            ++lineNumber;

            if (!headerRead)
            {
                headerRead = true;
                const auto fields = SplitTabs(line);
                u32 version = 0;
                if (fields.size() != 2 || fields[0] != kHeader || !ParseUnsigned(fields[1], version))
                {
                    AddDiagnostic(diagnostics, ResourceManifestDiagnosticCode::InvalidHeader,
                        lineNumber, {}, "Expected PYRAMID_RESOURCE_MANIFEST followed by a numeric version");
                    return false;
                }
                if (version != kResourceManifestVersion)
                {
                    AddDiagnostic(diagnostics, ResourceManifestDiagnosticCode::UnsupportedVersion,
                        lineNumber, {}, "Unsupported resource manifest version " + std::to_string(version));
                    return false;
                }
            }
            else if (!line.empty() && line.front() != '#')
            {
                const auto fields = SplitTabs(line);
                if (fields.size() != 4)
                {
                    AddDiagnostic(diagnostics, ResourceManifestDiagnosticCode::MalformedEntry,
                        lineNumber, {}, "Manifest entries require type, key, asset ID, and generation");
                }
                else
                {
                    ResourceType type = ResourceType::Mesh;
                    u64 high = 0;
                    u64 low = 0;
                    u32 generation = 0;
                    const std::string key(fields[1]);
                    if (!ParseType(fields[0], type))
                    {
                        AddDiagnostic(diagnostics, ResourceManifestDiagnosticCode::InvalidResourceType,
                            lineNumber, key, "Unknown resource type");
                    }
                    else if (!IsManifestKeyValid(fields[1]))
                    {
                        AddDiagnostic(diagnostics, ResourceManifestDiagnosticCode::InvalidKey,
                            lineNumber, key, "Resource keys may contain only letters, digits, '.', '_', '-', and '/'");
                    }
                    else if (parsed.Contains(fields[1]))
                    {
                        AddDiagnostic(diagnostics, ResourceManifestDiagnosticCode::DuplicateKey,
                            lineNumber, key, "Duplicate resource manifest key");
                    }
                    else if (!ParseAssetId(fields[2], high, low))
                    {
                        AddDiagnostic(diagnostics, ResourceManifestDiagnosticCode::InvalidAssetId,
                            lineNumber, key, "Asset ID must be a non-zero 32-digit hexadecimal value");
                    }
                    else if (!ParseUnsigned(fields[3], generation) || generation == 0)
                    {
                        AddDiagnostic(diagnostics, ResourceManifestDiagnosticCode::InvalidGeneration,
                            lineNumber, key, "Generation must be a non-zero 32-bit integer");
                    }
                    else
                    {
                        parsed.m_entries.push_back({key, type, high, low, generation, lineNumber});
                    }
                }
            }

            if (end == std::string_view::npos) break;
            offset = end + 1;
        }

        if (!headerRead)
        {
            AddDiagnostic(diagnostics, ResourceManifestDiagnosticCode::InvalidHeader,
                0, {}, "Resource manifest is empty");
            return false;
        }
        if (!diagnostics.empty()) return false;
        output = std::move(parsed);
        return true;
    }

    ResourceManifestRestoreReport ResourceManifest::Restore(const ResourceRegistry& registry) const
    {
        ResourceManifestRestoreReport report;
        for (const auto& entry : m_entries)
        {
            u32 currentGeneration = 0;
            switch (entry.type)
            {
            case ResourceType::Mesh:
                currentGeneration = registry.GetHandle(MeshAssetId{entry.assetIdHigh, entry.assetIdLow}).GetGeneration();
                break;
            case ResourceType::Shader:
                currentGeneration = registry.GetHandle(ShaderAssetId{entry.assetIdHigh, entry.assetIdLow}).GetGeneration();
                break;
            case ResourceType::Texture:
                currentGeneration = registry.GetHandle(TextureAssetId{entry.assetIdHigh, entry.assetIdLow}).GetGeneration();
                break;
            case ResourceType::Material:
                currentGeneration = registry.GetHandle(MaterialAssetId{entry.assetIdHigh, entry.assetIdLow}).GetGeneration();
                break;
            }

            if (currentGeneration == 0)
            {
                ++report.missingAssets;
                AddDiagnostic(report.diagnostics, ResourceManifestDiagnosticCode::MissingAsset,
                    entry.sourceLine, entry.key, "Resource asset is not resident in the registry");
            }
            else if (currentGeneration != entry.generation)
            {
                ++report.staleGenerations;
                AddDiagnostic(report.diagnostics, ResourceManifestDiagnosticCode::StaleGeneration,
                    entry.sourceLine, entry.key,
                    "Serialized generation " + std::to_string(entry.generation) +
                    " does not match current generation " + std::to_string(currentGeneration));
            }
            else
            {
                ++report.resolved;
            }
        }
        return report;
    }
}
