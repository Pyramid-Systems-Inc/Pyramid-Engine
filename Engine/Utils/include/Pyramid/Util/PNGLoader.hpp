#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace Pyramid
{
    namespace Util
    {
        struct ImageData; // Forward declaration

        /**
         * @brief PNG image loader implementation
         * 
         * This class implements PNG image loading as specified in the PNG specification.
         * It supports:
         * - Color types: Grayscale, Truecolor, Indexed, Grayscale+Alpha, Truecolor+Alpha
         * - Bit depths: 1, 2, 4, 8, 16 (depending on color type)
         * - Interlacing: None and Adam7
         * - All PNG filter types (None, Sub, Up, Average, Paeth)
         * - Critical chunks: IHDR, PLTE, IDAT, IEND
         * - Ancillary chunks: tRNS (transparency)
         */
        class PNGLoader
        {
        public:
            /**
             * @brief Load a PNG image from file
             * 
             * @param filepath Path to the PNG file
             * @return ImageData structure with loaded image, or empty structure on failure
             */
            static ImageData LoadFromFile(const std::string& filepath);

            /**
             * @brief Load a PNG image from memory buffer
             * 
             * @param data Pointer to PNG data in memory
             * @param size Size of the PNG data in bytes
             * @return ImageData structure with loaded image, or empty structure on failure
             */
            static ImageData LoadFromMemory(const uint8_t* data, size_t size);

            /**
             * @brief Get the last error message
             * @return Error message string
             */
            static const std::string& GetLastError();

        private:
            static std::string s_LastError;

            // PNG color types
            enum ColorType : uint8_t
            {
                GRAYSCALE = 0,
                TRUECOLOR = 2,
                INDEXED = 3,
                GRAYSCALE_ALPHA = 4,
                TRUECOLOR_ALPHA = 6
            };

            // PNG filter types
            enum FilterType : uint8_t
            {
                FILTER_NONE = 0,
                FILTER_SUB = 1,
                FILTER_UP = 2,
                FILTER_AVERAGE = 3,
                FILTER_PAETH = 4
            };

            // PNG chunk structure
            struct PNGChunk
            {
                uint32_t length;
                uint32_t type;
                std::vector<uint8_t> data;
                uint32_t crc;
            };

            // PNG header information
            struct PNGHeader
            {
                uint32_t width;
                uint32_t height;
                uint8_t bitDepth;
                uint8_t colorType;
                uint8_t compressionMethod;
                uint8_t filterMethod;
                uint8_t interlaceMethod;
            };

            /**
             * @brief Internal PNG loading implementation
             * 
             * @param data Pointer to PNG data
             * @param size Size of PNG data
             * @return ImageData structure
             */
            static ImageData LoadPNGInternal(const uint8_t* data, size_t size);

            /**
             * @brief Verify PNG signature
             * 
             * @param data Pointer to data
             * @param size Size of data
             * @return true if valid PNG signature
             */
            static bool VerifySignature(const uint8_t* data, size_t size);

            /**
             * @brief Read a PNG chunk from data
             * 
             * @param data Pointer to chunk data
             * @param size Remaining data size
             * @param offset Current offset in data
             * @param chunk Output chunk structure
             * @return true if chunk was read successfully
             */
            static bool ReadChunk(const uint8_t* data, size_t size, size_t& offset, PNGChunk& chunk);

            /**
             * @brief Parse IHDR chunk
             * 
             * @param chunk The IHDR chunk
             * @param header Output header structure
             * @return true if parsed successfully
             */
            static bool ParseIHDR(const PNGChunk& chunk, PNGHeader& header);

            /**
             * @brief Process image data and apply filters
             * 
             * @param idatData Concatenated IDAT chunk data
             * @param header PNG header information
             * @param palette Color palette (for indexed images)
             * @return Processed image data
             */
            static std::vector<uint8_t> ProcessImageData(const std::vector<uint8_t>& idatData,
                                                       const PNGHeader& header,
                                                       const std::vector<uint8_t>& palette);

            /**
             * @brief Apply PNG filters to scanlines
             * 
             * @param filteredData Raw filtered data from DEFLATE
             * @param header PNG header information
             * @return Unfiltered image data
             */
            static std::vector<uint8_t> ApplyFilters(const std::vector<uint8_t>& filteredData,
                                                   const PNGHeader& header);

            /**
             * @brief Convert image data to final format
             * 
             * @param imageData Raw image data
             * @param header PNG header information
             * @param palette Color palette (for indexed images)
             * @return Final image data in RGB/RGBA format
             */
            static std::vector<uint8_t> ConvertToFinalFormat(const std::vector<uint8_t>& imageData,
                                                           const PNGHeader& header,
                                                           const std::vector<uint8_t>& palette);

            /**
             * @brief Calculate bytes per pixel for a given color type and bit depth
             * 
             * @param colorType PNG color type
             * @param bitDepth PNG bit depth
             * @return Bytes per pixel
             */
            static int GetBytesPerPixel(ColorType colorType, uint8_t bitDepth);

            /**
             * @brief Calculate CRC32 checksum
             * 
             * @param data Pointer to data
             * @param length Length of data
             * @return CRC32 checksum
             */
            static uint32_t CalculateCRC32(const uint8_t* data, size_t length);

            /**
             * @brief Paeth predictor function for PNG filtering
             * 
             * @param a Left pixel value
             * @param b Above pixel value  
             * @param c Above-left pixel value
             * @return Predicted value
             */
            static uint8_t PaethPredictor(uint8_t a, uint8_t b, uint8_t c);

            /**
             * @brief Set the last error message
             * @param error Error message
             */
            static void SetError(const std::string& error);
        };

    } // namespace Util
} // namespace Pyramid
