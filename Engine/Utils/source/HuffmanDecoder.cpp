#include "Pyramid/Util/HuffmanDecoder.hpp"
#include "Pyramid/Util/BitReader.hpp"
#include "Pyramid/Util/Log.hpp"
#include <algorithm>
#include <vector>
#include <stdexcept>

namespace Pyramid
{
    namespace Util
    {
        HuffmanDecoder::HuffmanDecoder()
            : m_IsValid(false)
        {
        }

        HuffmanDecoder::~HuffmanDecoder()
        {
            Clear();
        }

        bool HuffmanDecoder::BuildTree(const uint8_t *codeLengths, int numSymbols)
        {
            return BuildCanonicalTree(codeLengths, numSymbols);
        }

        int HuffmanDecoder::DecodeSymbol(BitReader &bitReader)
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

        bool HuffmanDecoder::IsValid() const
        {
            return m_IsValid;
        }

        void HuffmanDecoder::Clear()
        {
            m_Root.reset();
            m_IsValid = false;
        }

        bool HuffmanDecoder::BuildFixedLiteralTree()
        {
            // Fixed literal/length tree as defined in RFC 1951
            std::vector<uint8_t> codeLengths(288);

            // Symbols 0-143: 8 bits
            for (int i = 0; i <= 143; ++i)
            {
                codeLengths[i] = 8;
            }

            // Symbols 144-255: 9 bits
            for (int i = 144; i <= 255; ++i)
            {
                codeLengths[i] = 9;
            }

            // Symbols 256-279: 7 bits
            for (int i = 256; i <= 279; ++i)
            {
                codeLengths[i] = 7;
            }

            // Symbols 280-287: 8 bits
            for (int i = 280; i <= 287; ++i)
            {
                codeLengths[i] = 8;
            }

            return BuildCanonicalTree(codeLengths.data(), 288);
        }

        bool HuffmanDecoder::BuildFixedDistanceTree()
        {
            // Fixed distance tree: all 32 symbols have 5-bit codes
            std::vector<uint8_t> codeLengths(32, 5);
            return BuildCanonicalTree(codeLengths.data(), 32);
        }

        bool HuffmanDecoder::BuildCanonicalTree(const uint8_t *codeLengths, int numSymbols)
        {
            Clear();

            // Count the number of codes for each code length
            std::vector<int> lengthCount(16, 0); // Max code length is 15
            for (int i = 0; i < numSymbols; ++i)
            {
                if (codeLengths[i] > 15)
                {
                    PYRAMID_LOG_ERROR("HuffmanDecoder: Invalid code length ", (int)codeLengths[i], " for symbol ", i);
                    return false;
                }
                if (codeLengths[i] > 0)
                {
                    lengthCount[codeLengths[i]]++;
                }
            }

            // Check if we have any codes at all
            bool hasAnyCodes = false;
            for (int i = 1; i <= 15; ++i)
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

            // Calculate the first code for each length
            std::vector<int> firstCode(16, 0);
            int code = 0;
            for (int bits = 1; bits <= 15; ++bits)
            {
                firstCode[bits] = code;
                code += lengthCount[bits];
                code <<= 1;
            }

            // Create the root node
            m_Root = std::make_unique<HuffmanNode>();

            // Assign codes to symbols and build the tree
            for (int symbol = 0; symbol < numSymbols; ++symbol)
            {
                int length = codeLengths[symbol];
                if (length > 0)
                {
                    uint32_t symbolCode = firstCode[length];
                    firstCode[length]++;

                    InsertSymbol(symbolCode, length, symbol);
                }
            }

            m_IsValid = true;
            return true;
        }

        void HuffmanDecoder::InsertSymbol(uint32_t code, int codeLength, int symbol)
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

    } // namespace Util
} // namespace Pyramid
