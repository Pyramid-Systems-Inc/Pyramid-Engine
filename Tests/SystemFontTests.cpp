#include <Pyramid/Font/Font.hpp>
#include <Pyramid/Platform/SystemFont.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    class CacheDirectory final
    {
    public:
        CacheDirectory()
        {
            const std::string token = std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count());
            m_path = std::filesystem::temp_directory_path() /
                ("pyramid-system-font-tests-" + token);
        }

        ~CacheDirectory()
        {
            std::error_code ignored;
            std::filesystem::remove_all(m_path, ignored);
        }

        const std::filesystem::path& Path() const { return m_path; }

    private:
        std::filesystem::path m_path;
    };

    Pyramid::Font::BakeOptions MakeBake(
        Pyramid::u32 width,
        Pyramid::u32 height,
        std::vector<Pyramid::Font::CharacterRange> ranges)
    {
        Pyramid::Font::BakeOptions bake;
        bake.pixelHeight = 64.0f;
        bake.atlasWidth = width;
        bake.atlasHeight = height;
        bake.oversample = 1;
        bake.padding = 1;
        bake.mode = Pyramid::Font::RasterMode::SignedDistanceField;
        bake.distanceRange = 10.0f;
        bake.fallbackCodepoint = U'?';
        bake.missingGlyphPolicy = Pyramid::Font::MissingGlyphPolicy::Skip;
        bake.ranges = std::move(ranges);
        return bake;
    }

    std::vector<char32_t> RequiredArabicCodepoints()
    {
        std::vector<char32_t> required = {
            static_cast<char32_t>(0x060C),
            static_cast<char32_t>(0x061B),
            static_cast<char32_t>(0x061F)};
        for (char32_t codepoint = 0x064B; codepoint <= 0x065F; ++codepoint)
        {
            required.push_back(codepoint);
        }
        for (char32_t codepoint = 0x0660; codepoint <= 0x0669; ++codepoint)
        {
            required.push_back(codepoint);
        }
        for (char32_t codepoint = 0xFE80; codepoint <= 0xFEF4; ++codepoint)
        {
            required.push_back(codepoint);
        }
        return required;
    }

    bool HasRequiredGlyphs(
        const Pyramid::Font::BakedFont& font,
        const std::vector<char32_t>& required,
        char32_t* missing = nullptr)
    {
        for (const char32_t codepoint : required)
        {
            if (!font.FindGlyph(codepoint))
            {
                if (missing)
                {
                    *missing = codepoint;
                }
                return false;
            }
        }
        return true;
    }

    struct SelectedFont
    {
        std::string family;
        Pyramid::Platform::SystemFontData source;
        Pyramid::Font::CachedBakeResult baked;
    };

    bool SelectFont(
        const std::vector<std::string_view>& candidates,
        const Pyramid::Font::BakeOptions& bake,
        const std::vector<char32_t>& required,
        const std::filesystem::path& cacheDirectory,
        SelectedFont& output)
    {
        output = {};
        for (const std::string_view candidate : candidates)
        {
            Pyramid::Platform::SystemFontRequest request;
            request.preferredFamilies.emplace_back(candidate);
            Pyramid::Platform::SystemFontData source;
            std::string sourceError;
            if (!Pyramid::Platform::LoadSystemFont(request, source, &sourceError))
            {
                std::cerr << "System font candidate `" << candidate
                          << "` was unavailable: " << sourceError << '\n';
                continue;
            }

            Pyramid::Font::CachedBakeResult baked =
                Pyramid::Font::LoadOrBakeProcessedFont(
                    source.bytes.data(), source.bytes.size(), bake, cacheDirectory);
            if (!baked.Succeeded())
            {
                std::cerr << "System font candidate `" << candidate
                          << "` failed processing";
                for (const Pyramid::Font::Diagnostic& diagnostic : baked.diagnostics)
                {
                    std::cerr << ": " << diagnostic.message;
                }
                std::cerr << '\n';
                continue;
            }
            char32_t missing = 0;
            if (!HasRequiredGlyphs(baked.font, required, &missing))
            {
                std::cerr << "System font candidate `" << candidate
                          << "` missed required codepoint U+" << std::hex
                          << std::uppercase << static_cast<Pyramid::u32>(missing)
                          << std::dec << '\n';
                continue;
            }

            output.family = source.resolvedFamily;
            output.source = std::move(source);
            output.baked = std::move(baked);
            return true;
        }
        return false;
    }
}

int main()
{
    try
    {
        Pyramid::Platform::SystemFontRequest missingRequest;
        missingRequest.preferredFamilies.emplace_back(
            "Pyramid Engine Deliberately Missing Font");
        Pyramid::Platform::SystemFontData missingFont;
        Require(
            !Pyramid::Platform::LoadSystemFont(missingRequest, missingFont, nullptr),
            "missing family silently resolved to a substituted Windows font");

        CacheDirectory cache;
        const Pyramid::Font::BakeOptions latinBake = MakeBake(
            2048,
            1536,
            {
                {U' ', static_cast<char32_t>(0x017F)},
                {static_cast<char32_t>(0x0370), static_cast<char32_t>(0x03FF)},
                {static_cast<char32_t>(0x2000), static_cast<char32_t>(0x206F)},
                {static_cast<char32_t>(0x20AC), static_cast<char32_t>(0x20AC)},
                {static_cast<char32_t>(0x2190), static_cast<char32_t>(0x21FF)},
                {static_cast<char32_t>(0x2713), static_cast<char32_t>(0x2713)}});
        const std::vector<char32_t> requiredLatin = {
            U'?', U'A', U'a', U'0', U'\u00E9', U'\u03A9'};
        SelectedFont latin;
        Require(
            SelectFont(
                {"Segoe UI", "Tahoma", "Arial"},
                latinBake,
                requiredLatin,
                cache.Path(),
                latin),
            "no installed Latin UI font passed parsing, coverage, and SDF baking");
        Require(!latin.baked.cacheHit, "first Latin system-font bake was a cache hit");
        const Pyramid::Font::CachedBakeResult latinCached =
            Pyramid::Font::LoadOrBakeProcessedFont(
                latin.source.bytes.data(),
                latin.source.bytes.size(),
                latinBake,
                cache.Path());
        Require(
            latinCached.Succeeded() && latinCached.cacheHit &&
                latinCached.cachePath == latin.baked.cachePath,
            "Latin system-font processed cache did not produce a stable hit");

        const Pyramid::Font::BakeOptions symbolBake = MakeBake(
            1024,
            1024,
            {
                {static_cast<char32_t>(0x2190), static_cast<char32_t>(0x21FF)},
                {static_cast<char32_t>(0x2713), static_cast<char32_t>(0x2713)}});
        const std::vector<char32_t> requiredSymbols = {
            U'\u2190', U'\u2191', U'\u2192', U'\u2193', U'\u2713'};
        SelectedFont symbols;
        Require(
            SelectFont(
                {"Segoe UI Symbol", "Segoe UI", "Arial"},
                symbolBake,
                requiredSymbols,
                cache.Path(),
                symbols),
            "no installed symbol font passed arrow/checkmark coverage and SDF baking");
        Require(!symbols.baked.cacheHit, "first symbol system-font bake was a cache hit");
        const Pyramid::Font::CachedBakeResult symbolsCached =
            Pyramid::Font::LoadOrBakeProcessedFont(
                symbols.source.bytes.data(),
                symbols.source.bytes.size(),
                symbolBake,
                cache.Path());
        Require(
            symbolsCached.Succeeded() && symbolsCached.cacheHit &&
                symbolsCached.cachePath == symbols.baked.cachePath,
            "symbol system-font processed cache did not produce a stable hit");

        const Pyramid::Font::BakeOptions arabicBake = MakeBake(
            2048,
            1024,
            {
                {static_cast<char32_t>(0x0600), static_cast<char32_t>(0x06FF)},
                {static_cast<char32_t>(0xFE80), static_cast<char32_t>(0xFEF4)}});
        const std::vector<char32_t> requiredArabic = RequiredArabicCodepoints();
        SelectedFont arabic;
        Require(
            SelectFont(
                {
                    "Cairo",
                    "Tahoma",
                    "Arial",
                    "Sakkal Majalla",
                    "Traditional Arabic",
                    "Simplified Arabic"},
                arabicBake,
                requiredArabic,
                cache.Path(),
                arabic),
            "no installed Arabic UI font passed presentation-form coverage and SDF baking");
        Require(!arabic.baked.cacheHit, "first Arabic system-font bake was a cache hit");
        const Pyramid::Font::CachedBakeResult arabicCached =
            Pyramid::Font::LoadOrBakeProcessedFont(
                arabic.source.bytes.data(),
                arabic.source.bytes.size(),
                arabicBake,
                cache.Path());
        Require(
            arabicCached.Succeeded() && arabicCached.cacheHit &&
                arabicCached.cachePath == arabic.baked.cachePath,
            "Arabic system-font processed cache did not produce a stable hit");

        std::cout << "System font tests passed: Latin=" << latin.family
                  << ", Arabic=" << arabic.family
                  << ", Symbols=" << symbols.family << '\n';
    }
    catch (const std::exception& error)
    {
        std::cerr << "SystemFontTests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
