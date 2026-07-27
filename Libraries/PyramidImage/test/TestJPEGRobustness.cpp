#include "Pyramid/Util/Image.hpp"
#include "Pyramid/Util/JPEGLoader.hpp"
#include "Fixtures/JPEGFixtures.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <vector>

using Pyramid::Tests::Fixtures::BaselineRGBJPEG;
using Pyramid::Tests::Fixtures::BaselineRGBJPEGSize;
using Pyramid::Tests::Fixtures::ProgressiveGrayJPEG;
using Pyramid::Tests::Fixtures::ProgressiveGrayJPEGSize;
using namespace Pyramid::Util;

namespace
{
    int Fail(const char* message)
    {
        std::cerr << "TestJPEGRobustness failure: " << message << '\n';
        return EXIT_FAILURE;
    }

    bool Rejects(const std::uint8_t* data, std::size_t size)
    {
        ImageData image = JPEGLoader::LoadFromMemory(data, size);
        if (image.Data)
        {
            Image::Free(image.Data);
            return false;
        }
        return !JPEGLoader::GetLastError().empty();
    }
}

int main()
{
    for (std::size_t size = 0; size < BaselineRGBJPEGSize; size += 37)
    {
        if (!Rejects(BaselineRGBJPEG, size))
        {
            return Fail("a truncated baseline JPEG was accepted");
        }
    }

    for (std::size_t size = 1; size < ProgressiveGrayJPEGSize; size += 19)
    {
        if (!Rejects(ProgressiveGrayJPEG, size))
        {
            return Fail("a truncated progressive JPEG was accepted");
        }
    }

    std::vector<std::uint8_t> unsupported(BaselineRGBJPEG, BaselineRGBJPEG + BaselineRGBJPEGSize);
    const std::array<std::uint8_t, 2> framePattern = {0xFF, 0xC0};
    const auto frame = std::search(
        unsupported.begin(),
        unsupported.end(),
        framePattern.begin(),
        framePattern.end());
    if (frame == unsupported.end())
    {
        return Fail("could not locate the baseline frame marker fixture");
    }
    frame[1] = static_cast<std::uint8_t>(0xC9); // Arithmetic-coded sequential frame.
    if (!Rejects(unsupported.data(), unsupported.size()))
    {
        return Fail("an arithmetic-coded JPEG frame was accepted");
    }

    std::vector<std::uint8_t> unsupportedPrecision(
        BaselineRGBJPEG,
        BaselineRGBJPEG + BaselineRGBJPEGSize);
    const auto precisionFrame = std::search(
        unsupportedPrecision.begin(),
        unsupportedPrecision.end(),
        framePattern.begin(),
        framePattern.end());
    precisionFrame[4] = 12;
    if (!Rejects(unsupportedPrecision.data(), unsupportedPrecision.size()))
    {
        return Fail("a 12-bit JPEG frame was accepted");
    }

    std::vector<std::uint8_t> oversized(
        BaselineRGBJPEG,
        BaselineRGBJPEG + BaselineRGBJPEGSize);
    const auto oversizedFrame = std::search(
        oversized.begin(),
        oversized.end(),
        framePattern.begin(),
        framePattern.end());
    oversizedFrame[5] = 0xFF;
    oversizedFrame[6] = 0xFF;
    oversizedFrame[7] = 0xFF;
    oversizedFrame[8] = 0xFF;
    if (!Rejects(oversized.data(), oversized.size()))
    {
        return Fail("a JPEG exceeding the coefficient allocation limit was accepted");
    }

    std::vector<std::uint8_t> invalidQuantization(
        BaselineRGBJPEG,
        BaselineRGBJPEG + BaselineRGBJPEGSize);
    const std::array<std::uint8_t, 2> quantizationPattern = {0xFF, 0xDB};
    const auto quantization = std::search(
        invalidQuantization.begin(),
        invalidQuantization.end(),
        quantizationPattern.begin(),
        quantizationPattern.end());
    if (quantization == invalidQuantization.end())
    {
        return Fail("could not locate the quantization table fixture");
    }
    quantization[5] = 0;
    if (!Rejects(invalidQuantization.data(), invalidQuantization.size()))
    {
        return Fail("a zero-valued quantization table was accepted");
    }

    std::cout << "JPEG malformed-input and unsupported-variant tests passed\n";
    return EXIT_SUCCESS;
}
