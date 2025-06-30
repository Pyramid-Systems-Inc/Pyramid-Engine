#pragma once

#include <cstdint>
#include <vector>
#include <memory>

namespace Pyramid
{
    namespace Util
    {
        class BitReader;

        /**
         * @brief A Huffman decoder for DEFLATE decompression
         * 
         * This class builds Huffman trees from code lengths and provides
         * efficient decoding of Huffman-encoded symbols. It supports both
         * the literal/length alphabet and the distance alphabet used in DEFLATE.
         */
        class HuffmanDecoder
        {
        public:
            /**
             * @brief Construct an empty Huffman decoder
             */
            HuffmanDecoder();

            /**
             * @brief Destructor
             */
            ~HuffmanDecoder();

            /**
             * @brief Build the Huffman tree from code lengths
             * 
             * @param codeLengths Array of code lengths for each symbol
             * @param numSymbols Number of symbols in the alphabet
             * @return true if the tree was built successfully, false otherwise
             */
            bool BuildTree(const uint8_t* codeLengths, int numSymbols);

            /**
             * @brief Decode a single symbol from the bit stream
             * 
             * @param bitReader The bit reader to read from
             * @return The decoded symbol, or -1 if decoding failed
             */
            int DecodeSymbol(BitReader& bitReader);

            /**
             * @brief Check if the decoder has a valid tree
             * @return true if the tree is valid, false otherwise
             */
            bool IsValid() const;

            /**
             * @brief Clear the decoder and free memory
             */
            void Clear();

            // Predefined Huffman trees for DEFLATE
            /**
             * @brief Build the fixed literal/length Huffman tree used in DEFLATE
             * @return true if successful, false otherwise
             */
            bool BuildFixedLiteralTree();

            /**
             * @brief Build the fixed distance Huffman tree used in DEFLATE
             * @return true if successful, false otherwise
             */
            bool BuildFixedDistanceTree();

        private:
            /**
             * @brief Internal node structure for the Huffman tree
             */
            struct HuffmanNode
            {
                int symbol;                           ///< Symbol value (-1 for internal nodes)
                std::unique_ptr<HuffmanNode> left;    ///< Left child
                std::unique_ptr<HuffmanNode> right;   ///< Right child

                HuffmanNode() : symbol(-1) {}
                HuffmanNode(int sym) : symbol(sym) {}
            };

            std::unique_ptr<HuffmanNode> m_Root;  ///< Root of the Huffman tree
            bool m_IsValid;                       ///< Whether the tree is valid

            /**
             * @brief Build the Huffman tree using the canonical Huffman algorithm
             * 
             * This implements the algorithm described in RFC 1951 for building
             * canonical Huffman trees from code lengths.
             * 
             * @param codeLengths Array of code lengths
             * @param numSymbols Number of symbols
             * @return true if successful, false otherwise
             */
            bool BuildCanonicalTree(const uint8_t* codeLengths, int numSymbols);

            /**
             * @brief Insert a symbol into the tree at the specified code
             * 
             * @param code The Huffman code for this symbol
             * @param codeLength Length of the code in bits
             * @param symbol The symbol to insert
             */
            void InsertSymbol(uint32_t code, int codeLength, int symbol);
        };

    } // namespace Util
} // namespace Pyramid
