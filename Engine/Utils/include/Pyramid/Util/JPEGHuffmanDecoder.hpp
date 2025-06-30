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
         * @brief JPEG-specific Huffman decoder
         * 
         * This class implements Huffman decoding specifically for JPEG images.
         * Unlike PNG, JPEG uses custom Huffman tables defined in DHT segments
         * and has separate tables for DC and AC coefficients.
         */
        class JPEGHuffmanDecoder
        {
        public:
            /**
             * @brief Construct an empty JPEG Huffman decoder
             */
            JPEGHuffmanDecoder();

            /**
             * @brief Destructor
             */
            ~JPEGHuffmanDecoder();

            /**
             * @brief Build the Huffman tree from JPEG DHT data
             * 
             * @param codeLengths Array of code lengths (16 bytes)
             * @param symbols Array of symbols in order
             * @param numSymbols Number of symbols
             * @return true if the tree was built successfully, false otherwise
             */
            bool BuildTree(const uint8_t* codeLengths, const uint8_t* symbols, int numSymbols);

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

        private:
            /**
             * @brief Internal node structure for the Huffman tree
             */
            struct HuffmanNode
            {
                int symbol;                           ///< Symbol value (-1 for internal nodes)
                std::unique_ptr<HuffmanNode> left;    ///< Left child (0 bit)
                std::unique_ptr<HuffmanNode> right;   ///< Right child (1 bit)

                HuffmanNode() : symbol(-1) {}
                HuffmanNode(int sym) : symbol(sym) {}
            };

            std::unique_ptr<HuffmanNode> m_Root;  ///< Root of the Huffman tree
            bool m_IsValid;                       ///< Whether the tree is valid

            /**
             * @brief Build the Huffman tree using JPEG canonical Huffman algorithm
             * 
             * @param codeLengths Array of code lengths (16 bytes)
             * @param symbols Array of symbols
             * @param numSymbols Number of symbols
             * @return true if successful, false otherwise
             */
            bool BuildJPEGCanonicalTree(const uint8_t* codeLengths, const uint8_t* symbols, int numSymbols);

            /**
             * @brief Insert a symbol into the tree at the specified code
             * 
             * @param code The Huffman code for this symbol
             * @param codeLength Length of the code in bits
             * @param symbol The symbol to insert
             */
            void InsertSymbol(uint32_t code, int codeLength, int symbol);
        };

        /**
         * @brief JPEG coefficient decoder
         * 
         * This class handles the decoding of JPEG coefficients using
         * DC and AC Huffman tables. It manages the DC prediction and
         * AC coefficient run-length encoding.
         */
        class JPEGCoefficientDecoder
        {
        public:
            /**
             * @brief Construct a JPEG coefficient decoder
             */
            JPEGCoefficientDecoder();

            /**
             * @brief Set the Huffman decoders for DC and AC coefficients
             * 
             * @param dcDecoder DC Huffman decoder
             * @param acDecoder AC Huffman decoder
             */
            void SetHuffmanDecoders(JPEGHuffmanDecoder* dcDecoder, JPEGHuffmanDecoder* acDecoder);

            /**
             * @brief Decode a single 8x8 block of coefficients
             * 
             * @param bitReader The bit reader for input data
             * @param coefficients Output array for 64 coefficients (in zigzag order)
             * @param componentId Component ID (for DC prediction)
             * @return true if successful, false otherwise
             */
            bool DecodeBlock(BitReader& bitReader, int16_t coefficients[64], int componentId);

            /**
             * @brief Reset DC predictors (called at restart intervals)
             */
            void ResetDCPredictors();

            /**
             * @brief Get the last error message
             * @return Error message string
             */
            const std::string& GetLastError() const;

        private:
            JPEGHuffmanDecoder* m_DCDecoder;    ///< DC Huffman decoder
            JPEGHuffmanDecoder* m_ACDecoder;    ///< AC Huffman decoder
            int16_t m_DCPredictors[3];          ///< DC predictors for each component (Y, Cb, Cr)
            std::string m_LastError;            ///< Last error message

            // JPEG zigzag order for 8x8 blocks
            static const int ZIGZAG_ORDER[64];

            /**
             * @brief Decode a DC coefficient
             * 
             * @param bitReader The bit reader
             * @param componentId Component ID (0=Y, 1=Cb, 2=Cr)
             * @return Decoded DC coefficient, or INT16_MIN on error
             */
            int16_t DecodeDC(BitReader& bitReader, int componentId);

            /**
             * @brief Decode AC coefficients
             * 
             * @param bitReader The bit reader
             * @param coefficients Output coefficient array (positions 1-63)
             * @return true if successful, false otherwise
             */
            bool DecodeAC(BitReader& bitReader, int16_t coefficients[64]);

            /**
             * @brief Read additional bits for coefficient magnitude
             * 
             * @param bitReader The bit reader
             * @param category Category (number of bits to read)
             * @return Coefficient value
             */
            int16_t ReadCoefficientBits(BitReader& bitReader, int category);

            /**
             * @brief Set the last error message
             * @param error Error message
             */
            void SetError(const std::string& error);
        };

    } // namespace Util
} // namespace Pyramid
