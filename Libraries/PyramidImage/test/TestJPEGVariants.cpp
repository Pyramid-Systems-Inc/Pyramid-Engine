#include "Pyramid/Util/Image.hpp"
#include "Pyramid/Util/JPEGLoader.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

using namespace Pyramid::Util;

namespace
{
    int Fail(const std::string& message)
    {
        std::cerr << "TestJPEGVariants failure: " << message << '\n';
        return EXIT_FAILURE;
    }

    bool IsDominant(
        const ImageData& image,
        int x,
        int y,
        int channel,
        int minimum,
        int maximumOther)
    {
        const std::size_t index =
            (static_cast<std::size_t>(y) * static_cast<std::size_t>(image.Width) +
             static_cast<std::size_t>(x)) * 3U;
        if (image.Data[index + static_cast<std::size_t>(channel)] < minimum)
        {
            return false;
        }
        for (int candidate = 0; candidate < 3; ++candidate)
        {
            if (candidate != channel && image.Data[index + static_cast<std::size_t>(candidate)] > maximumOther)
            {
                return false;
            }
        }
        return true;
    }

    bool ValidateQuadrants(const ImageData& image)
    {
        if (image.Width != 19 || image.Height != 17 || image.Channels != 3)
        {
            return false;
        }
        if (!IsDominant(image, 3, 3, 0, 180, 90))
        {
            return false;
        }
        if (!IsDominant(image, 15, 3, 1, 170, 110))
        {
            return false;
        }
        if (!IsDominant(image, 3, 13, 2, 170, 110))
        {
            return false;
        }

        const std::size_t yellow = (13U * 19U + 15U) * 3U;
        return image.Data[yellow] > 150 && image.Data[yellow + 1] > 150 && image.Data[yellow + 2] < 120;
    }

    bool DecodeAndValidateColor(const std::filesystem::path& path)
    {
        ImageData image = JPEGLoader::LoadFromFile(path.string());
        if (!image.Data)
        {
            std::cerr << JPEGLoader::GetLastError() << '\n';
            return false;
        }
        const bool valid = ValidateQuadrants(image);
        Image::Free(image.Data);
        return valid;
    }
}

int main()
{
    const std::filesystem::path fixtureRoot =
        std::filesystem::path(PYRAMID_TEST_SOURCE_DIR) / "Tests" / "Fixtures" / "JPEG";

    for (const char* filename : {
             "baseline-420.jpg",
             "baseline-422.jpg",
             "progressive-420.jpg",
             "restart-420.jpg"})
    {
        if (!DecodeAndValidateColor(fixtureRoot / filename))
        {
            return Fail(std::string("variant did not decode correctly: ") + filename);
        }
    }

    ImageData grayscale = JPEGLoader::LoadFromFile((fixtureRoot / "baseline-gray.jpg").string());
    if (!grayscale.Data)
    {
        std::cerr << JPEGLoader::GetLastError() << '\n';
        return Fail("baseline grayscale fixture did not decode");
    }
    const bool grayscaleValid =
        grayscale.Width == 13 && grayscale.Height == 11 && grayscale.Channels == 3 &&
        grayscale.Data[0] == grayscale.Data[1] && grayscale.Data[1] == grayscale.Data[2] &&
        grayscale.Data[30] == grayscale.Data[31] && grayscale.Data[31] == grayscale.Data[32] &&
        grayscale.Data[30] != grayscale.Data[0];
    Image::Free(grayscale.Data);
    if (!grayscaleValid)
    {
        return Fail("grayscale output was not normalized to varying RGB samples");
    }

    std::cout << "JPEG subsampling, progressive-color, grayscale, odd-size, and restart tests passed\n";
    return EXIT_SUCCESS;
}
