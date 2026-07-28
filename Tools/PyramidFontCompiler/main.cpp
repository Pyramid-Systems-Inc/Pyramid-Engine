#include <Pyramid/Font/Font.hpp>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
    void PrintUsage()
    {
        std::cerr
            << "Usage: PyramidFontCompiler <input.ttf> <output.pfont> "
               "[pixel-height] [atlas-size] [U+XXXX[-U+YYYY] ...]\n";
    }

    bool ParseCodepoint(std::string_view text, char32_t& output)
    {
        if (text.size() < 3 || text[0] != 'U' || text[1] != '+')
        {
            return false;
        }
        try
        {
            std::size_t consumed = 0;
            const unsigned long value = std::stoul(std::string(text.substr(2)), &consumed, 16);
            if (consumed != text.size() - 2 || value > 0x10FFFFUL ||
                (value >= 0xD800UL && value <= 0xDFFFUL))
            {
                return false;
            }
            output = static_cast<char32_t>(value);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool ParseRange(std::string_view text, Pyramid::Font::CharacterRange& output)
    {
        const std::size_t separator = text.find('-', 2);
        if (separator == std::string_view::npos)
        {
            if (!ParseCodepoint(text, output.first))
            {
                return false;
            }
            output.last = output.first;
            return true;
        }

        if (!ParseCodepoint(text.substr(0, separator), output.first) ||
            !ParseCodepoint(text.substr(separator + 1), output.last) ||
            output.first > output.last)
        {
            return false;
        }
        return true;
    }
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        PrintUsage();
        return EXIT_FAILURE;
    }

    Pyramid::Font::BakeOptions options;
    if (argc >= 4)
    {
        try
        {
            options.pixelHeight = std::stof(argv[3]);
        }
        catch (...)
        {
            std::cerr << "Invalid pixel height\n";
            return EXIT_FAILURE;
        }
    }
    if (argc >= 5)
    {
        try
        {
            const unsigned long size = std::stoul(argv[4]);
            if (size > 16384UL)
            {
                throw std::out_of_range("atlas");
            }
            options.atlasWidth = static_cast<Pyramid::u32>(size);
            options.atlasHeight = static_cast<Pyramid::u32>(size);
        }
        catch (...)
        {
            std::cerr << "Invalid atlas size\n";
            return EXIT_FAILURE;
        }
    }
    for (int argument = 5; argument < argc; ++argument)
    {
        Pyramid::Font::CharacterRange range;
        if (!ParseRange(argv[argument], range))
        {
            std::cerr << "Invalid Unicode range: " << argv[argument] << '\n';
            PrintUsage();
            return EXIT_FAILURE;
        }
        options.ranges.push_back(range);
    }

    const auto loaded = Pyramid::Font::LoadTrueTypeFile(argv[1]);
    for (const auto& diagnostic : loaded.diagnostics)
    {
        std::cerr << (diagnostic.severity == Pyramid::Font::DiagnosticSeverity::Error
            ? "error: " : "warning: ") << diagnostic.message << '\n';
    }
    if (!loaded.Succeeded())
    {
        return EXIT_FAILURE;
    }

    const auto baked = Pyramid::Font::BakeFont(loaded.face, options);
    for (const auto& diagnostic : baked.diagnostics)
    {
        std::cerr << (diagnostic.severity == Pyramid::Font::DiagnosticSeverity::Error
            ? "error: " : "warning: ") << diagnostic.message << '\n';
    }
    if (!baked.Succeeded())
    {
        return EXIT_FAILURE;
    }

    std::string error;
    if (!Pyramid::Font::SaveProcessedFontFile(baked.font, argv[2], &error))
    {
        std::cerr << "error: " << error << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "Compiled " << loaded.face.familyName << " at "
              << options.pixelHeight << " px into " << argv[2] << " ("
              << baked.font.glyphs.size() << " glyphs, "
              << baked.font.kerning.size() << " kerning pairs)\n";
    return EXIT_SUCCESS;
}
