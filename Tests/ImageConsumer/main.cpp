#include <Pyramid/Util/Image.hpp>
#include <Pyramid/Util/JPEGLoader.hpp>

#include <cstdint>
#include <iostream>

int main()
{
    constexpr std::uint8_t invalidJPEG[] = {0xFF, 0xD8, 0xFF, 0xD9};
    const Pyramid::Util::ImageData image =
        Pyramid::Util::JPEGLoader::LoadFromMemory(invalidJPEG, sizeof(invalidJPEG));

    if (image.Data != nullptr || Pyramid::Util::JPEGLoader::GetLastError().empty())
    {
        Pyramid::Util::Image::Free(image.Data);
        return 1;
    }

    std::cout << "Pyramid::Image package is linked and operational\n";
    return 0;
}
