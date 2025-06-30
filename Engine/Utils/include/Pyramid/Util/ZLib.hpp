#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace Pyramid
{
    namespace Util
    {
        /**
         * @brief ZLIB decompression implementation
         * 
         * This class implements ZLIB decompression as specified in RFC 1950.
         * ZLIB is a wrapper around DEFLATE that adds:
         * - A 2-byte header with compression method and flags
         * - An optional dictionary ID (not commonly used)
         * - The DEFLATE-compressed data
         * - A 4-byte Adler-32 checksum of the uncompressed data
         */
        class ZLib
        {
        public:
            /**
             * @brief Decompress ZLIB-compressed data
             * 
             * @param compressedData Pointer to the ZLIB-compressed data
             * @param compressedSize Size of the compressed data in bytes
             * @param decompressedData Output vector to store decompressed data
             * @return true if decompression was successful, false otherwise
             */
            static bool Decompress(const uint8_t* compressedData, size_t compressedSize,
                                 std::vector<uint8_t>& decompressedData);

            /**
             * @brief Get the last error message from the most recent operation
             * @return Error message string
             */
            static const std::string& GetLastError();

        private:
            static std::string s_LastError;

            /**
             * @brief Calculate Adler-32 checksum
             * 
             * @param data Pointer to the data
             * @param length Length of the data in bytes
             * @return Adler-32 checksum
             */
            static uint32_t CalculateAdler32(const uint8_t* data, size_t length);

            /**
             * @brief Set the last error message
             * @param error Error message
             */
            static void SetError(const std::string& error);
        };

    } // namespace Util
} // namespace Pyramid
