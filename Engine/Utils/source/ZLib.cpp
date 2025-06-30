#include "Pyramid/Util/ZLib.hpp"
#include "Pyramid/Util/Inflate.hpp"
#include "Pyramid/Util/Log.hpp"

namespace Pyramid
{
    namespace Util
    {
        std::string ZLib::s_LastError;

        bool ZLib::Decompress(const uint8_t* compressedData, size_t compressedSize,
                            std::vector<uint8_t>& decompressedData)
        {
            s_LastError.clear();
            decompressedData.clear();

            if (!compressedData || compressedSize < 6)
            {
                SetError("Invalid ZLIB data: too small");
                return false;
            }

            // Parse ZLIB header (2 bytes)
            uint8_t cmf = compressedData[0];  // Compression Method and Flags
            uint8_t flg = compressedData[1];  // Flags

            // Check compression method (lower 4 bits of CMF)
            uint8_t compressionMethod = cmf & 0x0F;
            if (compressionMethod != 8)  // 8 = DEFLATE
            {
                SetError("Unsupported ZLIB compression method");
                return false;
            }

            // Check compression info (upper 4 bits of CMF)
            uint8_t compressionInfo = (cmf >> 4) & 0x0F;
            if (compressionInfo > 7)  // Window size = 2^(compressionInfo + 8)
            {
                SetError("Invalid ZLIB compression info");
                return false;
            }

            // Verify header checksum
            uint16_t headerChecksum = (static_cast<uint16_t>(cmf) << 8) | flg;
            if (headerChecksum % 31 != 0)
            {
                SetError("Invalid ZLIB header checksum");
                return false;
            }

            // Check for preset dictionary (FDICT flag)
            bool hasPresetDict = (flg & 0x20) != 0;
            if (hasPresetDict)
            {
                SetError("ZLIB preset dictionaries are not supported");
                return false;
            }

            // Calculate the start of DEFLATE data and Adler-32 checksum
            size_t deflateStart = 2;  // After 2-byte header
            size_t deflateSize = compressedSize - 6;  // Minus header (2) and checksum (4)

            if (deflateSize == 0)
            {
                SetError("No DEFLATE data in ZLIB stream");
                return false;
            }

            // Extract Adler-32 checksum (last 4 bytes, big-endian)
            uint32_t expectedAdler32 = 
                (static_cast<uint32_t>(compressedData[compressedSize - 4]) << 24) |
                (static_cast<uint32_t>(compressedData[compressedSize - 3]) << 16) |
                (static_cast<uint32_t>(compressedData[compressedSize - 2]) << 8) |
                static_cast<uint32_t>(compressedData[compressedSize - 1]);

            // Decompress the DEFLATE data
            Inflate inflater;
            if (!inflater.Decompress(compressedData + deflateStart, deflateSize, decompressedData))
            {
                SetError("DEFLATE decompression failed: " + inflater.GetLastError());
                return false;
            }

            // Verify Adler-32 checksum
            uint32_t actualAdler32 = CalculateAdler32(decompressedData.data(), decompressedData.size());
            if (actualAdler32 != expectedAdler32)
            {
                SetError("ZLIB Adler-32 checksum mismatch");
                return false;
            }

            return true;
        }

        const std::string& ZLib::GetLastError()
        {
            return s_LastError;
        }

        uint32_t ZLib::CalculateAdler32(const uint8_t* data, size_t length)
        {
            // Adler-32 algorithm as specified in RFC 1950
            const uint32_t MOD_ADLER = 65521;
            uint32_t a = 1;
            uint32_t b = 0;

            for (size_t i = 0; i < length; ++i)
            {
                a = (a + data[i]) % MOD_ADLER;
                b = (b + a) % MOD_ADLER;
            }

            return (b << 16) | a;
        }

        void ZLib::SetError(const std::string& error)
        {
            s_LastError = error;
            PYRAMID_LOG_ERROR("ZLib: ", error);
        }

    } // namespace Util
} // namespace Pyramid
