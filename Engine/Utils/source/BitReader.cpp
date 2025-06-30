#include "Pyramid/Util/BitReader.hpp"
#include "Pyramid/Util/Log.hpp"
#include <stdexcept>
#include <algorithm>

namespace Pyramid
{
    namespace Util
    {
        BitReader::BitReader(const uint8_t* data, size_t size)
            : m_Data(data), m_Size(size), m_BytePosition(0), m_BitPosition(0)
        {
            if (!data && size > 0)
            {
                throw std::invalid_argument("BitReader: data pointer is null but size is non-zero");
            }
        }

        uint32_t BitReader::ReadBits(int numBits)
        {
            if (numBits < 1 || numBits > 32)
            {
                throw std::invalid_argument("BitReader::ReadBits: numBits must be between 1 and 32");
            }

            uint32_t result = 0;
            int bitsRead = 0;

            while (bitsRead < numBits)
            {
                // Check if we have more data
                if (m_BytePosition >= m_Size)
                {
                    throw std::runtime_error("BitReader::ReadBits: Not enough data available");
                }

                // Get the current byte
                uint8_t currentByte = m_Data[m_BytePosition];
                
                // Calculate how many bits we can read from this byte
                int bitsAvailableInByte = 8 - m_BitPosition;
                int bitsToRead = std::min(numBits - bitsRead, bitsAvailableInByte);

                // Extract the bits we need from the current byte
                // DEFLATE uses LSB-first bit ordering within each byte
                uint8_t mask = (1 << bitsToRead) - 1;
                uint8_t bits = (currentByte >> m_BitPosition) & mask;

                // Add these bits to our result
                result |= (static_cast<uint32_t>(bits) << bitsRead);

                // Update positions
                bitsRead += bitsToRead;
                m_BitPosition += bitsToRead;

                // If we've consumed all bits in this byte, move to the next
                if (m_BitPosition >= 8)
                {
                    m_BytePosition++;
                    m_BitPosition = 0;
                }
            }

            return result;
        }

        uint32_t BitReader::ReadBit()
        {
            return ReadBits(1);
        }

        uint32_t BitReader::ReadBitsReversed(int numBits)
        {
            uint32_t value = ReadBits(numBits);
            return ReverseBits(value, numBits);
        }

        void BitReader::AlignToByte()
        {
            if (m_BitPosition != 0)
            {
                m_BytePosition++;
                m_BitPosition = 0;
            }
        }

        void BitReader::ReadBytes(uint8_t* buffer, size_t numBytes)
        {
            if (!buffer)
            {
                throw std::invalid_argument("BitReader::ReadBytes: buffer is null");
            }

            // Align to byte boundary first
            AlignToByte();

            // Check if we have enough bytes
            if (m_BytePosition + numBytes > m_Size)
            {
                throw std::runtime_error("BitReader::ReadBytes: Not enough data available");
            }

            // Copy the bytes
            std::copy(m_Data + m_BytePosition, m_Data + m_BytePosition + numBytes, buffer);
            m_BytePosition += numBytes;
        }

        bool BitReader::HasMoreBits() const
        {
            return m_BytePosition < m_Size || (m_BytePosition == m_Size && m_BitPosition < 8);
        }

        size_t BitReader::GetBitPosition() const
        {
            return m_BytePosition * 8 + m_BitPosition;
        }

        size_t BitReader::GetBytePosition() const
        {
            return m_BytePosition;
        }

        size_t BitReader::GetTotalSize() const
        {
            return m_Size;
        }

        uint32_t BitReader::ReverseBits(uint32_t value, int numBits) const
        {
            uint32_t result = 0;
            for (int i = 0; i < numBits; ++i)
            {
                if (value & (1u << i))
                {
                    result |= (1u << (numBits - 1 - i));
                }
            }
            return result;
        }

    } // namespace Util
} // namespace Pyramid
