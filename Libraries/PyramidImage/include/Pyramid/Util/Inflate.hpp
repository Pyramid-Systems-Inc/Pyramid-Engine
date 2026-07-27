#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace Pyramid
{
    namespace Util
    {
        class BitReader;
        class HuffmanDecoder;

        /**
         * @brief DEFLATE decompression implementation
         *
         * This class implements the DEFLATE decompression algorithm as specified
         * in RFC 1951. It handles all three block types:
         * - Uncompressed blocks
         * - Fixed Huffman blocks
         * - Dynamic Huffman blocks
         *
         * The class also implements the LZ77 sliding window for handling
         * back-references (length, distance pairs).
         */
        class Inflate
        {
        public:
            /**
             * @brief Construct an Inflate decompressor
             */
            Inflate();

            /**
             * @brief Destructor
             */
            ~Inflate();

            /**
             * @brief Decompress DEFLATE-compressed data
             *
             * @param compressedData Pointer to the compressed data
             * @param compressedSize Size of the compressed data in bytes
             * @param decompressedData Output vector to store decompressed data
             * @return true if decompression was successful, false otherwise
             */
            bool Decompress(const uint8_t *compressedData, size_t compressedSize,
                            std::vector<uint8_t> &decompressedData);

            /**
             * @brief Get the last error message
             * @return Error message string
             */
            const std::string &GetLastError() const;

        private:
            std::string m_LastError;                 ///< Last error message
            std::vector<uint8_t> m_SlidingWindow;    ///< LZ77 sliding window
            size_t m_WindowPos;                      ///< Current position in sliding window
            static const size_t WINDOW_SIZE = 32768; ///< Size of sliding window (32KB)

            // Length and distance tables for DEFLATE
            static const int LENGTH_CODES[];
            static const int LENGTH_EXTRA_BITS[];
            static const int DISTANCE_CODES[];
            static const int DISTANCE_EXTRA_BITS[];

            /**
             * @brief Process a single DEFLATE block
             *
             * @param bitReader The bit reader for input data
             * @param output Output vector for decompressed data
             * @return true if successful, false otherwise
             */
            bool ProcessBlock(BitReader &bitReader, std::vector<uint8_t> &output);

            /**
             * @brief Process an uncompressed block
             *
             * @param bitReader The bit reader for input data
             * @param output Output vector for decompressed data
             * @return true if successful, false otherwise
             */
            bool ProcessUncompressedBlock(BitReader &bitReader, std::vector<uint8_t> &output);

            /**
             * @brief Process a block with fixed Huffman codes
             *
             * @param bitReader The bit reader for input data
             * @param output Output vector for decompressed data
             * @return true if successful, false otherwise
             */
            bool ProcessFixedHuffmanBlock(BitReader &bitReader, std::vector<uint8_t> &output);

            /**
             * @brief Process a block with dynamic Huffman codes
             *
             * @param bitReader The bit reader for input data
             * @param output Output vector for decompressed data
             * @return true if successful, false otherwise
             */
            bool ProcessDynamicHuffmanBlock(BitReader &bitReader, std::vector<uint8_t> &output);

            /**
             * @brief Decode dynamic Huffman trees
             *
             * @param bitReader The bit reader for input data
             * @param literalDecoder Output literal/length decoder
             * @param distanceDecoder Output distance decoder
             * @return true if successful, false otherwise
             */
            bool DecodeDynamicHuffmanTrees(BitReader &bitReader,
                                           HuffmanDecoder &literalDecoder,
                                           HuffmanDecoder &distanceDecoder);

            /**
             * @brief Process literal/length and distance codes
             *
             * @param bitReader The bit reader for input data
             * @param literalDecoder Literal/length Huffman decoder
             * @param distanceDecoder Distance Huffman decoder
             * @param output Output vector for decompressed data
             * @return true if successful, false otherwise
             */
            bool ProcessLiteralLengthDistance(BitReader &bitReader,
                                              HuffmanDecoder &literalDecoder,
                                              HuffmanDecoder &distanceDecoder,
                                              std::vector<uint8_t> &output);

            /**
             * @brief Add a byte to the sliding window and output
             *
             * @param byte The byte to add
             * @param output Output vector
             */
            void AddByte(uint8_t byte, std::vector<uint8_t> &output);

            /**
             * @brief Copy bytes from the sliding window (for LZ77 back-references)
             *
             * @param distance Distance back in the window
             * @param length Number of bytes to copy
             * @param output Output vector
             * @return true if successful, false if invalid distance/length
             */
            bool CopyFromWindow(int distance, int length, std::vector<uint8_t> &output);

            /**
             * @brief Set the last error message
             * @param error Error message
             */
            void SetError(const std::string &error);
        };

    } // namespace Util
} // namespace Pyramid
