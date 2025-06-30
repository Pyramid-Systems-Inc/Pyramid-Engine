#include "Pyramid/Util/JPEGLoader.hpp"
#include "Pyramid/Util/Image.hpp"
#include "Pyramid/Util/Log.hpp"
#include <fstream>
#include <cstring>

namespace Pyramid
{
    namespace Util
    {
        std::string JPEGLoader::s_LastError;

        ImageData JPEGLoader::LoadFromFile(const std::string &filepath)
        {
            std::ifstream file(filepath, std::ios::binary);
            if (!file.is_open())
            {
                SetError("Failed to open JPEG file: " + filepath);
                return ImageData{};
            }

            // Get file size
            file.seekg(0, std::ios::end);
            size_t fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            // Read entire file into memory
            std::vector<uint8_t> fileData(fileSize);
            file.read(reinterpret_cast<char *>(fileData.data()), fileSize);

            if (!file)
            {
                SetError("Failed to read JPEG file: " + filepath);
                return ImageData{};
            }

            return LoadFromMemory(fileData.data(), fileSize);
        }

        ImageData JPEGLoader::LoadFromMemory(const uint8_t *data, size_t size)
        {
            return LoadJPEGInternal(data, size);
        }

        const std::string &JPEGLoader::GetLastError()
        {
            return s_LastError;
        }

        ImageData JPEGLoader::LoadJPEGInternal(const uint8_t *data, size_t size)
        {
            s_LastError.clear();

            if (!data || size < 4)
            {
                SetError("Invalid JPEG data");
                return ImageData{};
            }

            // Check JPEG signature (SOI marker)
            if (ReadBigEndian16(data) != SOI)
            {
                SetError("Invalid JPEG signature");
                return ImageData{};
            }

            JPEGDecoder decoder = {};
            const uint8_t *imageDataStart = nullptr;
            size_t imageDataSize = 0;

            if (!ParseJPEGMarkers(data, size, decoder, imageDataStart, imageDataSize))
            {
                return ImageData{};
            }

            // For now, return basic image info (we'll implement full decoding in subsequent tasks)
            ImageData result;
            result.Width = decoder.frameHeader.width;
            result.Height = decoder.frameHeader.height;
            result.Channels = (decoder.frameHeader.numComponents == 1) ? 1 : 3; // Grayscale or RGB

            // Allocate dummy data for now (will be replaced with actual decoding)
            size_t dataSize = result.Width * result.Height * result.Channels;
            result.Data = new unsigned char[dataSize];
            memset(result.Data, 128, dataSize); // Fill with gray for testing

            PYRAMID_LOG_INFO("Successfully parsed JPEG image: ", result.Width, "x", result.Height, " (", result.Channels, " channels)");
            return result;
        }

        bool JPEGLoader::ParseJPEGMarkers(const uint8_t *data, size_t size,
                                          JPEGDecoder &decoder,
                                          const uint8_t *&imageDataStart,
                                          size_t &imageDataSize)
        {
            size_t offset = 2; // Skip SOI marker

            while (offset < size - 1)
            {
                // Read marker
                uint16_t marker = ReadBigEndian16(data + offset);
                offset += 2;

                if ((marker & 0xFF00) != 0xFF00)
                {
                    SetError("Invalid JPEG marker");
                    return false;
                }

                switch (marker)
                {
                case SOI:
                    // Start of Image - already handled
                    break;

                case EOI:
                    // End of Image
                    return true;

                case SOF0:
                {
                    // Start of Frame (Baseline DCT)
                    if (offset + 2 > size)
                    {
                        SetError("Truncated SOF0 segment");
                        return false;
                    }

                    uint16_t segmentLength = ReadBigEndian16(data + offset);
                    offset += 2;

                    if (offset + segmentLength - 2 > size)
                    {
                        SetError("Truncated SOF0 segment data");
                        return false;
                    }

                    if (!ParseSOF0(data + offset, segmentLength - 2, decoder.frameHeader))
                    {
                        return false;
                    }

                    offset += segmentLength - 2;
                    break;
                }

                case DQT:
                {
                    // Define Quantization Table
                    if (offset + 2 > size)
                    {
                        SetError("Truncated DQT segment");
                        return false;
                    }

                    uint16_t segmentLength = ReadBigEndian16(data + offset);
                    offset += 2;

                    if (offset + segmentLength - 2 > size)
                    {
                        SetError("Truncated DQT segment data");
                        return false;
                    }

                    if (!ParseDQT(data + offset, segmentLength - 2, decoder.quantTables))
                    {
                        return false;
                    }

                    offset += segmentLength - 2;
                    break;
                }

                case DHT:
                {
                    // Define Huffman Table
                    if (offset + 2 > size)
                    {
                        SetError("Truncated DHT segment");
                        return false;
                    }

                    uint16_t segmentLength = ReadBigEndian16(data + offset);
                    offset += 2;

                    if (offset + segmentLength - 2 > size)
                    {
                        SetError("Truncated DHT segment data");
                        return false;
                    }

                    if (!ParseDHT(data + offset, segmentLength - 2, decoder.dcTables, decoder.acTables))
                    {
                        return false;
                    }

                    offset += segmentLength - 2;
                    break;
                }

                case SOS:
                {
                    // Start of Scan
                    if (offset + 2 > size)
                    {
                        SetError("Truncated SOS segment");
                        return false;
                    }

                    uint16_t segmentLength = ReadBigEndian16(data + offset);
                    offset += 2;

                    if (offset + segmentLength - 2 > size)
                    {
                        SetError("Truncated SOS segment data");
                        return false;
                    }

                    if (!ParseSOS(data + offset, segmentLength - 2, decoder.scanHeader))
                    {
                        return false;
                    }

                    offset += segmentLength - 2;

                    // Image data starts after SOS
                    imageDataStart = data + offset;
                    imageDataSize = size - offset;

                    // For now, we'll stop parsing here
                    // In a full implementation, we'd continue to find EOI
                    return true;
                }

                case DRI:
                {
                    // Define Restart Interval
                    if (offset + 4 > size)
                    {
                        SetError("Truncated DRI segment");
                        return false;
                    }

                    uint16_t segmentLength = ReadBigEndian16(data + offset);
                    offset += 2;

                    if (segmentLength != 4)
                    {
                        SetError("Invalid DRI segment length");
                        return false;
                    }

                    decoder.restartInterval = ReadBigEndian16(data + offset);
                    offset += 2;
                    break;
                }

                default:
                    // Skip unknown markers
                    if (offset + 2 > size)
                    {
                        SetError("Truncated marker segment");
                        return false;
                    }

                    uint16_t segmentLength = ReadBigEndian16(data + offset);
                    offset += 2;

                    if (offset + segmentLength - 2 > size)
                    {
                        SetError("Truncated marker segment data");
                        return false;
                    }

                    offset += segmentLength - 2;
                    break;
                }
            }

            SetError("Unexpected end of JPEG data");
            return false;
        }

        uint16_t JPEGLoader::ReadBigEndian16(const uint8_t *data)
        {
            return (static_cast<uint16_t>(data[0]) << 8) | data[1];
        }

        bool JPEGLoader::ParseSOF0(const uint8_t *data, size_t size, FrameHeader &frameHeader)
        {
            if (size < 6)
            {
                SetError("SOF0 segment too small");
                return false;
            }

            frameHeader.precision = data[0];
            frameHeader.height = ReadBigEndian16(data + 1);
            frameHeader.width = ReadBigEndian16(data + 3);
            frameHeader.numComponents = data[5];

            if (frameHeader.precision != 8)
            {
                SetError("Unsupported JPEG precision (only 8-bit supported)");
                return false;
            }

            if (frameHeader.numComponents < 1 || frameHeader.numComponents > 3)
            {
                SetError("Unsupported number of components");
                return false;
            }

            if (size < 6 + frameHeader.numComponents * 3)
            {
                SetError("SOF0 segment too small for component data");
                return false;
            }

            for (int i = 0; i < frameHeader.numComponents; ++i)
            {
                frameHeader.components[i].id = data[6 + i * 3];
                frameHeader.components[i].samplingFactor = data[6 + i * 3 + 1];
                frameHeader.components[i].quantTableId = data[6 + i * 3 + 2];

                if (frameHeader.components[i].quantTableId > 3)
                {
                    SetError("Invalid quantization table ID");
                    return false;
                }
            }

            return true;
        }

        bool JPEGLoader::ParseDQT(const uint8_t *data, size_t size, QuantizationTable quantTables[4])
        {
            size_t offset = 0;

            while (offset < size)
            {
                if (offset + 1 > size)
                {
                    SetError("Truncated DQT table header");
                    return false;
                }

                uint8_t tableInfo = data[offset++];
                uint8_t precision = (tableInfo >> 4) & 0x0F;
                uint8_t tableId = tableInfo & 0x0F;

                if (tableId > 3)
                {
                    SetError("Invalid quantization table ID");
                    return false;
                }

                if (precision > 1)
                {
                    SetError("Invalid quantization table precision");
                    return false;
                }

                size_t tableSize = (precision == 0) ? 64 : 128;
                if (offset + tableSize > size)
                {
                    SetError("Truncated DQT table data");
                    return false;
                }

                quantTables[tableId].defined = true;
                quantTables[tableId].precision = precision;

                if (precision == 0)
                {
                    // 8-bit values
                    for (int i = 0; i < 64; ++i)
                    {
                        quantTables[tableId].values[i] = data[offset + i];
                    }
                }
                else
                {
                    // 16-bit values
                    for (int i = 0; i < 64; ++i)
                    {
                        quantTables[tableId].values[i] = ReadBigEndian16(data + offset + i * 2);
                    }
                }

                offset += tableSize;
            }

            return true;
        }

        bool JPEGLoader::ParseDHT(const uint8_t *data, size_t size,
                                  HuffmanTable dcTables[4], HuffmanTable acTables[4])
        {
            size_t offset = 0;

            while (offset < size)
            {
                if (offset + 1 > size)
                {
                    SetError("Truncated DHT table header");
                    return false;
                }

                uint8_t tableInfo = data[offset++];
                uint8_t tableClass = (tableInfo >> 4) & 0x0F; // 0 = DC, 1 = AC
                uint8_t tableId = tableInfo & 0x0F;

                if (tableId > 3)
                {
                    SetError("Invalid Huffman table ID");
                    return false;
                }

                if (tableClass > 1)
                {
                    SetError("Invalid Huffman table class");
                    return false;
                }

                if (offset + 16 > size)
                {
                    SetError("Truncated DHT code lengths");
                    return false;
                }

                HuffmanTable *table = (tableClass == 0) ? &dcTables[tableId] : &acTables[tableId];
                table->defined = true;

                // Read code lengths
                uint16_t totalSymbols = 0;
                for (int i = 0; i < 16; ++i)
                {
                    table->codeLengths[i] = data[offset + i];
                    totalSymbols += table->codeLengths[i];
                }
                offset += 16;

                if (totalSymbols > 256)
                {
                    SetError("Too many Huffman symbols");
                    return false;
                }

                if (offset + totalSymbols > size)
                {
                    SetError("Truncated DHT symbols");
                    return false;
                }

                // Read symbols
                table->numSymbols = totalSymbols;
                for (int i = 0; i < totalSymbols; ++i)
                {
                    table->symbols[i] = data[offset + i];
                }
                offset += totalSymbols;
            }

            return true;
        }

        bool JPEGLoader::ParseSOS(const uint8_t *data, size_t size, ScanHeader &scanHeader)
        {
            if (size < 1)
            {
                SetError("SOS segment too small");
                return false;
            }

            scanHeader.numComponents = data[0];

            if (scanHeader.numComponents < 1 || scanHeader.numComponents > 3)
            {
                SetError("Invalid number of components in scan");
                return false;
            }

            if (size < 1 + scanHeader.numComponents * 2 + 3)
            {
                SetError("SOS segment too small for component data");
                return false;
            }

            for (int i = 0; i < scanHeader.numComponents; ++i)
            {
                scanHeader.components[i].id = data[1 + i * 2];
                scanHeader.components[i].huffmanTableId = data[1 + i * 2 + 1];
            }

            size_t offset = 1 + scanHeader.numComponents * 2;
            scanHeader.spectralStart = data[offset];
            scanHeader.spectralEnd = data[offset + 1];
            scanHeader.approximation = data[offset + 2];

            // For baseline JPEG, these should be specific values
            if (scanHeader.spectralStart != 0 || scanHeader.spectralEnd != 63 || scanHeader.approximation != 0)
            {
                SetError("Non-baseline JPEG not supported");
                return false;
            }

            return true;
        }

        void JPEGLoader::SetError(const std::string &error)
        {
            s_LastError = error;
            PYRAMID_LOG_ERROR("JPEGLoader: ", error);
        }

    } // namespace Util
} // namespace Pyramid
