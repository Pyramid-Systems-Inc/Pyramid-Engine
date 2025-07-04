#include "Pyramid/Util/JPEGHuffmanDecoder.hpp"
#include "Pyramid/Util/BitReader.hpp"
#include "Pyramid/Util/Log.hpp"
#include <algorithm>
#include <climits>

namespace Pyramid
{
    namespace Util
    {
        // JPEG zigzag order for 8x8 DCT blocks
        const int JPEGCoefficientDecoder::ZIGZAG_ORDER[64] = {
            0, 1, 8, 16, 9, 2, 3, 10,
            17, 24, 32, 25, 18, 11, 4, 5,
            12, 19, 26, 33, 40, 48, 41, 34,
            27, 20, 13, 6, 7, 14, 21, 28,
            35, 42, 49, 56, 57, 50, 43, 36,
            29, 22, 15, 23, 30, 37, 44, 51,
            58, 59, 52, 45, 38, 31, 39, 46,
            53, 60, 61, 54, 47, 55, 62, 63};

        // JPEGHuffmanDecoder Implementation
        JPEGHuffmanDecoder::JPEGHuffmanDecoder()
            : m_IsValid(false)
        {
        }

        JPEGHuffmanDecoder::~JPEGHuffmanDecoder()
        {
            Clear();
        }

        bool JPEGHuffmanDecoder::BuildTree(const uint8_t *codeLengths, const uint8_t *symbols, int numSymbols)
        {
            return BuildJPEGCanonicalTree(codeLengths, symbols, numSymbols);
        }

        int JPEGHuffmanDecoder::DecodeSymbol(BitReader &bitReader)
        {
            if (!m_IsValid || !m_Root)
            {
                return -1;
            }

            HuffmanNode *current = m_Root.get();

            while (current->symbol == -1) // While we're at an internal node
            {
                if (!bitReader.HasMoreBits())
                {
                    return -1; // Not enough data
                }

                uint32_t bit = bitReader.ReadBit();

                if (bit == 0)
                {
                    current = current->left.get();
                }
                else
                {
                    current = current->right.get();
                }

                if (!current)
                {
                    return -1; // Invalid tree structure
                }
            }

            return current->symbol;
        }

        bool JPEGHuffmanDecoder::IsValid() const
        {
            return m_IsValid;
        }

        void JPEGHuffmanDecoder::Clear()
        {
            m_Root.reset();
            m_IsValid = false;
        }

        bool JPEGHuffmanDecoder::BuildJPEGCanonicalTree(const uint8_t *codeLengths, const uint8_t *symbols, int numSymbols)
        {
            Clear();

            // Count the number of codes for each code length
            std::vector<int> lengthCount(17, 0); // JPEG uses lengths 1-16
            for (int i = 0; i < 16; ++i)
            {
                lengthCount[i + 1] = codeLengths[i];
            }

            // Check if we have any codes at all
            bool hasAnyCodes = false;
            for (int i = 1; i <= 16; ++i)
            {
                if (lengthCount[i] > 0)
                {
                    hasAnyCodes = true;
                    break;
                }
            }

            if (!hasAnyCodes)
            {
                // Empty tree is valid for some cases
                m_IsValid = true;
                return true;
            }

            // Verify that we have the correct number of symbols
            int totalSymbols = 0;
            for (int i = 1; i <= 16; ++i)
            {
                totalSymbols += lengthCount[i];
            }

            if (totalSymbols != numSymbols)
            {
                PYRAMID_LOG_ERROR("JPEGHuffmanDecoder: Symbol count mismatch");
                return false;
            }

            // Calculate the first code for each length
            std::vector<int> firstCode(17, 0);
            int code = 0;
            for (int bits = 1; bits <= 16; ++bits)
            {
                firstCode[bits] = code;
                code += lengthCount[bits];
                code <<= 1;
            }

            // Create the root node
            m_Root = std::make_unique<HuffmanNode>();

            // Assign codes to symbols and build the tree
            int symbolIndex = 0;
            for (int length = 1; length <= 16; ++length)
            {
                for (int i = 0; i < lengthCount[length]; ++i)
                {
                    if (symbolIndex >= numSymbols)
                    {
                        PYRAMID_LOG_ERROR("JPEGHuffmanDecoder: Not enough symbols");
                        return false;
                    }

                    uint32_t symbolCode = firstCode[length] + i;
                    int symbol = symbols[symbolIndex++];

                    InsertSymbol(symbolCode, length, symbol);
                }
            }

            m_IsValid = true;
            return true;
        }

        void JPEGHuffmanDecoder::InsertSymbol(uint32_t code, int codeLength, int symbol)
        {
            HuffmanNode *current = m_Root.get();

            // Traverse the tree based on the code bits (MSB first)
            for (int i = codeLength - 1; i >= 0; --i)
            {
                uint32_t bit = (code >> i) & 1;

                if (bit == 0)
                {
                    if (!current->left)
                    {
                        current->left = std::make_unique<HuffmanNode>();
                    }
                    current = current->left.get();
                }
                else
                {
                    if (!current->right)
                    {
                        current->right = std::make_unique<HuffmanNode>();
                    }
                    current = current->right.get();
                }
            }

            // Set the symbol at the leaf node
            current->symbol = symbol;
        }

        // JPEGCoefficientDecoder Implementation
        JPEGCoefficientDecoder::JPEGCoefficientDecoder()
            : m_DCDecoder(nullptr), m_ACDecoder(nullptr)
        {
            ResetDCPredictors();
        }

        void JPEGCoefficientDecoder::SetHuffmanDecoders(JPEGHuffmanDecoder *dcDecoder, JPEGHuffmanDecoder *acDecoder)
        {
            m_DCDecoder = dcDecoder;
            m_ACDecoder = acDecoder;
        }

        bool JPEGCoefficientDecoder::DecodeBlock(BitReader &bitReader, int16_t coefficients[64], int componentId)
        {
            if (!m_DCDecoder || !m_ACDecoder)
            {
                SetError("Huffman decoders not set");
                return false;
            }

            // Clear the coefficient array
            std::fill(coefficients, coefficients + 64, 0);

            // Decode DC coefficient
            int16_t dcValue = DecodeDC(bitReader, componentId);
            if (dcValue == INT16_MIN)
            {
                return false;
            }
            coefficients[0] = dcValue;

            // Decode AC coefficients
            if (!DecodeAC(bitReader, coefficients))
            {
                return false;
            }

            return true;
        }

        void JPEGCoefficientDecoder::ResetDCPredictors()
        {
            m_DCPredictors[0] = 0; // Y component
            m_DCPredictors[1] = 0; // Cb component
            m_DCPredictors[2] = 0; // Cr component
        }

        const std::string &JPEGCoefficientDecoder::GetLastError() const
        {
            return m_LastError;
        }

        int16_t JPEGCoefficientDecoder::DecodeDC(BitReader &bitReader, int componentId)
        {
            // Decode the DC category (number of additional bits)
            int category = m_DCDecoder->DecodeSymbol(bitReader);
            if (category < 0)
            {
                SetError("Failed to decode DC category");
                return INT16_MIN;
            }

            if (category > 11) // DC categories are 0-11 for 8-bit precision
            {
                SetError("Invalid DC category");
                return INT16_MIN;
            }

            // Read the additional bits for the DC coefficient
            int16_t dcDiff = 0;
            if (category > 0)
            {
                dcDiff = ReadCoefficientBits(bitReader, category);
            }

            // Apply DC prediction
            if (componentId < 0 || componentId > 2)
            {
                componentId = 0; // Default to Y component
            }

            m_DCPredictors[componentId] += dcDiff;
            return m_DCPredictors[componentId];
        }

        bool JPEGCoefficientDecoder::DecodeAC(BitReader &bitReader, int16_t coefficients[64])
        {
            int position = 1; // Start after DC coefficient

            while (position < 64)
            {
                // Decode AC symbol (run/size combination)
                int symbol = m_ACDecoder->DecodeSymbol(bitReader);
                if (symbol < 0)
                {
                    SetError("Failed to decode AC symbol");
                    return false;
                }

                // Special case: End of Block (EOB)
                if (symbol == 0x00)
                {
                    break; // All remaining coefficients are zero
                }

                // Extract run length and category
                int runLength = (symbol >> 4) & 0x0F; // Upper 4 bits
                int category = symbol & 0x0F;         // Lower 4 bits

                // Special case: ZRL (Zero Run Length) - 16 zeros
                if (symbol == 0xF0)
                {
                    position += 16;
                    if (position >= 64)
                    {
                        SetError("AC run length exceeded block size");
                        return false;
                    }
                    continue;
                }

                // Skip zeros
                position += runLength;
                if (position >= 64)
                {
                    SetError("AC position exceeded block size");
                    return false;
                }

                // Read the AC coefficient value
                if (category == 0)
                {
                    SetError("Invalid AC category (0 with non-zero run)");
                    return false;
                }

                int16_t acValue = ReadCoefficientBits(bitReader, category);

                // Store in zigzag order
                coefficients[ZIGZAG_ORDER[position]] = acValue;
                position++;
            }

            return true;
        }

        int16_t JPEGCoefficientDecoder::ReadCoefficientBits(BitReader &bitReader, int category)
        {
            if (category == 0)
            {
                return 0;
            }

            if (category > 16)
            {
                SetError("Invalid coefficient category");
                return 0;
            }

            // Read the magnitude bits
            uint32_t magnitude = bitReader.ReadBits(category);

            // Check if the coefficient is positive or negative
            // If the MSB is 1, it's positive; if 0, it's negative
            uint32_t signBit = 1u << (category - 1);

            if (magnitude & signBit)
            {
                // Positive coefficient
                return static_cast<int16_t>(magnitude);
            }
            else
            {
                // Negative coefficient: subtract (2^category - 1) from magnitude
                int16_t negativeValue = static_cast<int16_t>(magnitude) - static_cast<int16_t>((1 << category) - 1);
                return negativeValue;
            }
        }

        void JPEGCoefficientDecoder::SetError(const std::string &error)
        {
            m_LastError = error;
            PYRAMID_LOG_ERROR("JPEGCoefficientDecoder: ", error);
        }

    } // namespace Util
} // namespace Pyramid
