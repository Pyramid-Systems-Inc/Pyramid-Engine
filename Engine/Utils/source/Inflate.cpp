#include "Pyramid/Util/Inflate.hpp"
#include "Pyramid/Util/BitReader.hpp"
#include "Pyramid/Util/HuffmanDecoder.hpp"
#include "Pyramid/Util/Log.hpp"
#include <stdexcept>
#include <algorithm>

namespace Pyramid
{
    namespace Util
    {
        // Length codes and extra bits for DEFLATE (RFC 1951)
        const int Inflate::LENGTH_CODES[] = {
            3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
            35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};

        const int Inflate::LENGTH_EXTRA_BITS[] = {
            0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
            3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};

        // Distance codes and extra bits for DEFLATE (RFC 1951)
        const int Inflate::DISTANCE_CODES[] = {
            1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
            257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
            8193, 12289, 16385, 24577};

        const int Inflate::DISTANCE_EXTRA_BITS[] = {
            0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
            7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

        Inflate::Inflate()
            : m_WindowPos(0)
        {
            m_SlidingWindow.resize(WINDOW_SIZE);
        }

        Inflate::~Inflate()
        {
        }

        bool Inflate::Decompress(const uint8_t *compressedData, size_t compressedSize,
                                 std::vector<uint8_t> &decompressedData)
        {
            if (!compressedData || compressedSize == 0)
            {
                SetError("Invalid input data");
                return false;
            }

            // Clear previous state
            decompressedData.clear();
            m_SlidingWindow.assign(WINDOW_SIZE, 0);
            m_WindowPos = 0;
            m_LastError.clear();

            BitReader bitReader(compressedData, compressedSize);

            try
            {
                bool isFinalBlock = false;
                while (!isFinalBlock)
                {
                    // Read block header
                    if (!bitReader.HasMoreBits())
                    {
                        SetError("Unexpected end of data while reading block header");
                        return false;
                    }

                    isFinalBlock = bitReader.ReadBit() != 0;

                    if (!ProcessBlock(bitReader, decompressedData))
                    {
                        return false;
                    }
                }

                return true;
            }
            catch (const std::exception &e)
            {
                SetError(std::string("Exception during decompression: ") + e.what());
                return false;
            }
        }

        const std::string &Inflate::GetLastError() const
        {
            return m_LastError;
        }

        bool Inflate::ProcessBlock(BitReader &bitReader, std::vector<uint8_t> &output)
        {
            if (!bitReader.HasMoreBits())
            {
                SetError("Unexpected end of data while reading block type");
                return false;
            }

            // Read block type (2 bits)
            uint32_t blockType = bitReader.ReadBits(2);

            switch (blockType)
            {
            case 0: // Uncompressed block
                return ProcessUncompressedBlock(bitReader, output);

            case 1: // Fixed Huffman block
                return ProcessFixedHuffmanBlock(bitReader, output);

            case 2: // Dynamic Huffman block
                return ProcessDynamicHuffmanBlock(bitReader, output);

            case 3: // Reserved (error)
                SetError("Invalid block type (reserved)");
                return false;

            default:
                SetError("Unknown block type");
                return false;
            }
        }

        bool Inflate::ProcessUncompressedBlock(BitReader &bitReader, std::vector<uint8_t> &output)
        {
            // Align to byte boundary
            bitReader.AlignToByte();

            // Read length and its complement
            if (bitReader.GetBytePosition() + 4 > bitReader.GetTotalSize())
            {
                SetError("Not enough data for uncompressed block header");
                return false;
            }

            uint8_t lengthBytes[4];
            bitReader.ReadBytes(lengthBytes, 4);

            uint16_t length = lengthBytes[0] | (lengthBytes[1] << 8);
            uint16_t nlength = lengthBytes[2] | (lengthBytes[3] << 8);

            // Verify that nlength is the one's complement of length
            if ((length ^ nlength) != 0xFFFF)
            {
                SetError("Invalid uncompressed block: length check failed");
                return false;
            }

            // Read the uncompressed data
            if (bitReader.GetBytePosition() + length > bitReader.GetTotalSize())
            {
                SetError("Not enough data for uncompressed block content");
                return false;
            }

            for (int i = 0; i < length; ++i)
            {
                uint8_t byte;
                bitReader.ReadBytes(&byte, 1);
                AddByte(byte, output);
            }

            return true;
        }

        bool Inflate::ProcessFixedHuffmanBlock(BitReader &bitReader, std::vector<uint8_t> &output)
        {
            HuffmanDecoder literalDecoder;
            HuffmanDecoder distanceDecoder;

            if (!literalDecoder.BuildFixedLiteralTree())
            {
                SetError("Failed to build fixed literal Huffman tree");
                return false;
            }

            if (!distanceDecoder.BuildFixedDistanceTree())
            {
                SetError("Failed to build fixed distance Huffman tree");
                return false;
            }

            return ProcessLiteralLengthDistance(bitReader, literalDecoder, distanceDecoder, output);
        }

        void Inflate::AddByte(uint8_t byte, std::vector<uint8_t> &output)
        {
            output.push_back(byte);
            m_SlidingWindow[m_WindowPos] = byte;
            m_WindowPos = (m_WindowPos + 1) % WINDOW_SIZE;
        }

        bool Inflate::ProcessDynamicHuffmanBlock(BitReader &bitReader, std::vector<uint8_t> &output)
        {
            HuffmanDecoder literalDecoder;
            HuffmanDecoder distanceDecoder;

            if (!DecodeDynamicHuffmanTrees(bitReader, literalDecoder, distanceDecoder))
            {
                return false;
            }

            return ProcessLiteralLengthDistance(bitReader, literalDecoder, distanceDecoder, output);
        }

        bool Inflate::DecodeDynamicHuffmanTrees(BitReader &bitReader,
                                                HuffmanDecoder &literalDecoder,
                                                HuffmanDecoder &distanceDecoder)
        {
            // Read tree sizes
            uint32_t hlit = bitReader.ReadBits(5) + 257; // Number of literal/length codes
            uint32_t hdist = bitReader.ReadBits(5) + 1;  // Number of distance codes
            uint32_t hclen = bitReader.ReadBits(4) + 4;  // Number of code length codes

            if (hlit > 286 || hdist > 30 || hclen > 19)
            {
                SetError("Invalid dynamic Huffman tree parameters");
                return false;
            }

            // Code length code order (as specified in RFC 1951)
            static const int CODE_LENGTH_ORDER[] = {
                16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

            // Read code lengths for the code length alphabet
            std::vector<uint8_t> codeLengthLengths(19, 0);
            for (uint32_t i = 0; i < hclen; ++i)
            {
                codeLengthLengths[CODE_LENGTH_ORDER[i]] = bitReader.ReadBits(3);
            }

            // Build the code length Huffman tree
            HuffmanDecoder codeLengthDecoder;
            if (!codeLengthDecoder.BuildTree(codeLengthLengths.data(), 19))
            {
                SetError("Failed to build code length Huffman tree");
                return false;
            }

            // Decode the literal/length and distance code lengths
            std::vector<uint8_t> allCodeLengths;
            allCodeLengths.reserve(hlit + hdist);

            while (allCodeLengths.size() < hlit + hdist)
            {
                int symbol = codeLengthDecoder.DecodeSymbol(bitReader);
                if (symbol < 0)
                {
                    SetError("Failed to decode code length symbol");
                    return false;
                }

                if (symbol < 16)
                {
                    // Literal code length
                    allCodeLengths.push_back(symbol);
                }
                else if (symbol == 16)
                {
                    // Repeat previous code length 3-6 times
                    if (allCodeLengths.empty())
                    {
                        SetError("Invalid repeat code (no previous length)");
                        return false;
                    }
                    uint32_t repeat = bitReader.ReadBits(2) + 3;
                    uint8_t prevLength = allCodeLengths.back();
                    for (uint32_t i = 0; i < repeat && allCodeLengths.size() < hlit + hdist; ++i)
                    {
                        allCodeLengths.push_back(prevLength);
                    }
                }
                else if (symbol == 17)
                {
                    // Repeat 0 for 3-10 times
                    uint32_t repeat = bitReader.ReadBits(3) + 3;
                    for (uint32_t i = 0; i < repeat && allCodeLengths.size() < hlit + hdist; ++i)
                    {
                        allCodeLengths.push_back(0);
                    }
                }
                else if (symbol == 18)
                {
                    // Repeat 0 for 11-138 times
                    uint32_t repeat = bitReader.ReadBits(7) + 11;
                    for (uint32_t i = 0; i < repeat && allCodeLengths.size() < hlit + hdist; ++i)
                    {
                        allCodeLengths.push_back(0);
                    }
                }
                else
                {
                    SetError("Invalid code length symbol");
                    return false;
                }
            }

            // Split into literal/length and distance code lengths
            std::vector<uint8_t> literalLengths(allCodeLengths.begin(), allCodeLengths.begin() + hlit);
            std::vector<uint8_t> distanceLengths(allCodeLengths.begin() + hlit, allCodeLengths.end());

            // Build the trees
            if (!literalDecoder.BuildTree(literalLengths.data(), hlit))
            {
                SetError("Failed to build literal/length Huffman tree");
                return false;
            }

            if (!distanceDecoder.BuildTree(distanceLengths.data(), hdist))
            {
                SetError("Failed to build distance Huffman tree");
                return false;
            }

            return true;
        }

        bool Inflate::ProcessLiteralLengthDistance(BitReader &bitReader,
                                                   HuffmanDecoder &literalDecoder,
                                                   HuffmanDecoder &distanceDecoder,
                                                   std::vector<uint8_t> &output)
        {
            while (true)
            {
                int symbol = literalDecoder.DecodeSymbol(bitReader);
                if (symbol < 0)
                {
                    SetError("Failed to decode literal/length symbol");
                    return false;
                }

                if (symbol < 256)
                {
                    // Literal byte
                    AddByte(static_cast<uint8_t>(symbol), output);
                }
                else if (symbol == 256)
                {
                    // End of block
                    break;
                }
                else if (symbol <= 285)
                {
                    // Length code
                    int lengthIndex = symbol - 257;
                    if (lengthIndex >= 29)
                    {
                        SetError("Invalid length code");
                        return false;
                    }

                    int length = LENGTH_CODES[lengthIndex];
                    int extraBits = LENGTH_EXTRA_BITS[lengthIndex];
                    if (extraBits > 0)
                    {
                        length += bitReader.ReadBits(extraBits);
                    }

                    // Decode distance
                    int distanceSymbol = distanceDecoder.DecodeSymbol(bitReader);
                    if (distanceSymbol < 0 || distanceSymbol >= 30)
                    {
                        SetError("Invalid distance symbol");
                        return false;
                    }

                    int distance = DISTANCE_CODES[distanceSymbol];
                    int distanceExtraBits = DISTANCE_EXTRA_BITS[distanceSymbol];
                    if (distanceExtraBits > 0)
                    {
                        distance += bitReader.ReadBits(distanceExtraBits);
                    }

                    // Copy from sliding window
                    if (!CopyFromWindow(distance, length, output))
                    {
                        return false;
                    }
                }
                else
                {
                    SetError("Invalid literal/length symbol");
                    return false;
                }
            }

            return true;
        }

        bool Inflate::CopyFromWindow(int distance, int length, std::vector<uint8_t> &output)
        {
            if (distance <= 0 || distance > static_cast<int>(WINDOW_SIZE))
            {
                SetError("Invalid copy distance");
                return false;
            }

            if (length <= 0)
            {
                SetError("Invalid copy length");
                return false;
            }

            // Calculate the starting position in the sliding window
            size_t startPos;
            if (distance <= static_cast<int>(m_WindowPos))
            {
                startPos = m_WindowPos - distance;
            }
            else
            {
                startPos = WINDOW_SIZE - (distance - m_WindowPos);
            }

            // Copy bytes one by one (handles overlapping copies correctly)
            for (int i = 0; i < length; ++i)
            {
                uint8_t byte = m_SlidingWindow[startPos];
                AddByte(byte, output);
                startPos = (startPos + 1) % WINDOW_SIZE;
            }

            return true;
        }

        void Inflate::SetError(const std::string &error)
        {
            m_LastError = error;
            PYRAMID_LOG_ERROR("Inflate: ", error);
        }

    } // namespace Util
} // namespace Pyramid
