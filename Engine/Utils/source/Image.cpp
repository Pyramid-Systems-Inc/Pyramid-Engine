#include "Pyramid/Util/Image.hpp"
#include "Pyramid/Core/Log.hpp"
#include <fstream>
#include <vector>

namespace Pyramid
{
    namespace Util
    {

// TGA file header structure.
// #pragma pack(push, 1) ensures the compiler doesn't add padding bytes,
// so the struct's memory layout matches the file format exactly.
#pragma pack(push, 1)
        struct TGAHeader
        {
            uint8_t idLength;
            uint8_t colorMapType;
            uint8_t imageType;
            uint8_t colorMapSpec[5];
            uint16_t xOrigin;
            uint16_t yOrigin;
            uint16_t width;
            uint16_t height;
            uint8_t bpp; // bits per pixel
            uint8_t imageDescriptor;
        };
#pragma pack(pop)

        ImageData Image::Load(const std::string &filepath)
        {
            ImageData result;
            std::ifstream file(filepath, std::ios::binary);

            if (!file.is_open())
            {
                PYRAMID_LOG_ERROR("Failed to open image file: ", filepath);
                return result;
            }

            TGAHeader header;
            file.read(reinterpret_cast<char *>(&header), sizeof(header));

            if (!file)
            {
                PYRAMID_LOG_ERROR("Failed to read TGA header from: ", filepath);
                return result;
            }

            // We only support uncompressed true-color images (type 2)
            if (header.imageType != 2)
            {
                PYRAMID_LOG_ERROR("Unsupported TGA image type: ", (int)header.imageType, ". Only type 2 is supported. File: ", filepath);
                return result;
            }

            // We only support 24 (RGB) and 32 (RGBA) bits per pixel
            if (header.bpp != 24 && header.bpp != 32)
            {
                PYRAMID_LOG_ERROR("Unsupported TGA bpp: ", (int)header.bpp, ". Only 24 or 32 is supported. File: ", filepath);
                return result;
            }

            result.Width = header.width;
            result.Height = header.height;
            result.Channels = header.bpp / 8;

            // Skip the image ID field if it exists
            if (header.idLength > 0)
            {
                file.seekg(header.idLength, std::ios::cur);
            }

            uint32_t imageSize = result.Width * result.Height * result.Channels;
            result.Data = new unsigned char[imageSize];
            file.read(reinterpret_cast<char *>(result.Data), imageSize);

            if (!file)
            {
                PYRAMID_LOG_ERROR("Failed to read TGA image data from: ", filepath);
                delete[] result.Data;
                result.Data = nullptr;
                return result;
            }

            // TGA stores images in BGR(A) format, so we need to swap the B and R channels for OpenGL (which expects RGB(A))
            for (uint32_t i = 0; i < imageSize; i += result.Channels)
            {
                std::swap(result.Data[i], result.Data[i + 2]);
            }

            // TGA can be stored top-down or bottom-up. Bit 5 of imageDescriptor tells us.
            // 0 = bottom-up (OpenGL's preferred format), 1 = top-down.
            bool isTopDown = (header.imageDescriptor & 0x20) != 0;
            if (isTopDown)
            {
                uint32_t bytesPerRow = result.Width * result.Channels;
                unsigned char *tempRow = new unsigned char[bytesPerRow];
                for (int i = 0; i < result.Height / 2; ++i)
                {
                    unsigned char *row1 = result.Data + i * bytesPerRow;
                    unsigned char *row2 = result.Data + (result.Height - 1 - i) * bytesPerRow;
                    memcpy(tempRow, row1, bytesPerRow);
                    memcpy(row1, row2, bytesPerRow);
                    memcpy(row2, tempRow, bytesPerRow);
                }
                delete[] tempRow;
            }

            PYRAMID_LOG_INFO("Successfully loaded TGA image: ", filepath, " (", result.Width, "x", result.Height, ")");
            return result;
        }

        void Image::Free(unsigned char *data)
        {
            if (data)
            {
                delete[] data;
            }
        }

    }
} // namespace Pyramid::Util