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

        ImageData JPEGLoader::LoadFromFile(const std::string& filepath)
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
            file.read(reinterpret_cast<char*>(fileData.data()), fileSize);

            if (!file)
            {
                SetError("Failed to read JPEG file: " + filepath);
                return ImageData{};
            }

            return LoadFromMemory(fileData.data(), fileSize);
        }

        ImageData JPEGLoader::LoadFromMemory(const uint8_t* data, size_t size)
        {
            return LoadJPEGInternal(data, size);
        }

        const std::string& JPEGLoader::GetLastError()
        {
            return s_LastError;
        }

        ImageData JPEGLoader::LoadJPEGInternal(const uint8_t* data, size_t size)
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
            const uint8_t* imageDataStart = nullptr;
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

        bool JPEGLoader::ParseJPEGMarkers(const uint8_t* data, size_t size, 
                                        JPEGDecoder& decoder,
                                        const uint8_t*& imageDataStart,
                                        size_t& imageDataSize)
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

        uint16_t JPEGLoader::ReadBigEndian16(const uint8_t* data)
        {
            return (static_cast<uint16_t>(data[0]) << 8) | data[1];
        }

        void JPEGLoader::SetError(const std::string& error)
        {
            s_LastError = error;
            PYRAMID_LOG_ERROR("JPEGLoader: ", error);
        }

    } // namespace Util
} // namespace Pyramid
