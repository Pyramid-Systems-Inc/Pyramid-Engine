#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace Pyramid::Util
{
    struct ImageData;

    /**
     * Decode JPEG images through PyramidImage's engine-owned decoder.
     *
     * The loader always returns tightly packed 8-bit RGB pixels. Both baseline
     * and progressive Huffman JPEG streams are accepted. Arithmetic-coded, lossless,
     * 12-bit, hierarchical, and four-component JPEG variants fail explicitly. Memory
     * returned in ImageData is owned by Pyramid and must be released with
     * Image::Free().
     */
    class JPEGLoader
    {
    public:
        static ImageData LoadFromFile(const std::string& filepath);
        static ImageData LoadFromMemory(const std::uint8_t* data, std::size_t size);
        static const std::string& GetLastError();

    private:
        static ImageData Decode(const std::uint8_t* data, std::size_t size);
        static void SetError(const std::string& error);

        static thread_local std::string s_LastError;
    };
} // namespace Pyramid::Util
