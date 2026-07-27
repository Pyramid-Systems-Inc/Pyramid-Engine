#pragma once

#include <cstdint>
#include <cstddef>

namespace Pyramid
{
    namespace Util
    {
        /**
         * @brief A utility class for reading data bit-by-bit from a byte stream.
         * 
         * This class is designed specifically for DEFLATE decompression, where data
         * needs to be read in variable-length bit sequences. It handles the bit ordering
         * and endianness requirements of the DEFLATE format.
         * 
         * DEFLATE bit ordering:
         * - Bits are read from least significant to most significant within each byte
         * - Multi-byte values are stored in little-endian format
         */
        class BitReader
        {
        public:
            /**
             * @brief Construct a BitReader from a byte buffer
             * @param data Pointer to the byte data
             * @param size Size of the data in bytes
             */
            BitReader(const uint8_t* data, size_t size);

            /**
             * @brief Read a specified number of bits from the stream
             * @param numBits Number of bits to read (1-32)
             * @return The bits read as a 32-bit unsigned integer
             * @throws std::runtime_error if not enough bits are available
             */
            uint32_t ReadBits(int numBits);

            /**
             * @brief Read a single bit from the stream
             * @return The bit value (0 or 1)
             * @throws std::runtime_error if no more bits are available
             */
            uint32_t ReadBit();

            /**
             * @brief Read bits and return them in reverse bit order
             * 
             * This is useful for reading Huffman codes where the bit order
             * needs to be reversed for tree traversal.
             * 
             * @param numBits Number of bits to read (1-32)
             * @return The bits read with reversed bit order
             */
            uint32_t ReadBitsReversed(int numBits);

            /**
             * @brief Align the bit position to the next byte boundary
             * 
             * This discards any remaining bits in the current byte and
             * moves to the start of the next byte.
             */
            void AlignToByte();

            /**
             * @brief Read raw bytes from the current byte-aligned position
             * @param buffer Buffer to store the read bytes
             * @param numBytes Number of bytes to read
             * @throws std::runtime_error if not enough bytes are available
             */
            void ReadBytes(uint8_t* buffer, size_t numBytes);

            /**
             * @brief Check if there are more bits available to read
             * @return true if more bits are available, false otherwise
             */
            bool HasMoreBits() const;

            /**
             * @brief Get the current bit position in the stream
             * @return Current bit position (0-based)
             */
            size_t GetBitPosition() const;

            /**
             * @brief Get the current byte position in the stream
             * @return Current byte position (0-based)
             */
            size_t GetBytePosition() const;

            /**
             * @brief Get the total size of the data in bytes
             * @return Total data size in bytes
             */
            size_t GetTotalSize() const;

        private:
            const uint8_t* m_Data;      ///< Pointer to the byte data
            size_t m_Size;              ///< Size of the data in bytes
            size_t m_BytePosition;      ///< Current byte position
            int m_BitPosition;          ///< Current bit position within the byte (0-7)

            /**
             * @brief Reverse the bits in a value
             * @param value The value to reverse
             * @param numBits Number of bits to consider
             * @return The value with reversed bits
             */
            uint32_t ReverseBits(uint32_t value, int numBits) const;
        };

    } // namespace Util
} // namespace Pyramid
