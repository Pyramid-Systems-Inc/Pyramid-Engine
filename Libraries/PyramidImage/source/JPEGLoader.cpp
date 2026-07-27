#include "Pyramid/Util/JPEGLoader.hpp"

#include "Pyramid/Util/Image.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Pyramid::Util
{
    namespace
    {
        constexpr std::size_t MaxInputBytes = 512ULL * 1024ULL * 1024ULL;
        constexpr std::size_t MaxDecodedImageBytes = 512ULL * 1024ULL * 1024ULL;
        constexpr std::size_t MaxCoefficientBytes = 512ULL * 1024ULL * 1024ULL;
        constexpr double Pi = 3.141592653589793238462643383279502884;

        constexpr std::array<std::uint8_t, 64> ZigZag = {
            0, 1, 8, 16, 9, 2, 3, 10,
            17, 24, 32, 25, 18, 11, 4, 5,
            12, 19, 26, 33, 40, 48, 41, 34,
            27, 20, 13, 6, 7, 14, 21, 28,
            35, 42, 49, 56, 57, 50, 43, 36,
            29, 22, 15, 23, 30, 37, 44, 51,
            58, 59, 52, 45, 38, 31, 39, 46,
            53, 60, 61, 54, 47, 55, 62, 63};

        enum class FrameType
        {
            None,
            Baseline,
            Progressive
        };

        struct QuantizationTable
        {
            std::array<std::uint16_t, 64> values{};
            bool valid = false;
        };

        class EntropyReader;

        struct HuffmanTable
        {
            std::array<std::uint8_t, 17> counts{};
            std::array<std::uint16_t, 17> firstCode{};
            std::array<std::uint16_t, 17> firstSymbol{};
            std::array<std::uint8_t, 256> symbols{};
            std::uint16_t symbolCount = 0;
            bool valid = false;

            bool Build(
                const std::array<std::uint8_t, 17>& lengths,
                const std::uint8_t* sourceSymbols,
                std::size_t sourceSymbolCount,
                std::string& error)
            {
                counts = lengths;
                symbolCount = static_cast<std::uint16_t>(sourceSymbolCount);
                std::copy_n(sourceSymbols, sourceSymbolCount, symbols.begin());

                std::uint32_t code = 0;
                std::uint32_t symbolIndex = 0;
                for (std::uint32_t length = 1; length <= 16; ++length)
                {
                    if (code + counts[length] > (1U << length))
                    {
                        error = "JPEG Huffman table is oversubscribed";
                        return false;
                    }

                    firstCode[length] = static_cast<std::uint16_t>(code);
                    firstSymbol[length] = static_cast<std::uint16_t>(symbolIndex);
                    symbolIndex += counts[length];
                    code = (code + counts[length]) << 1U;
                }

                valid = true;
                return true;
            }

            bool Decode(EntropyReader& reader, std::uint8_t& symbol, std::string& error) const;
        };

        struct Component
        {
            std::uint8_t id = 0;
            std::uint8_t horizontalSampling = 0;
            std::uint8_t verticalSampling = 0;
            std::uint8_t quantizationTable = 0;
            std::size_t actualBlockColumns = 0;
            std::size_t actualBlockRows = 0;
            std::size_t paddedBlockColumns = 0;
            std::size_t paddedBlockRows = 0;
            std::vector<std::int32_t> coefficients;
            std::vector<std::uint8_t> samples;
        };

        struct ScanComponent
        {
            std::size_t componentIndex = 0;
            std::uint8_t dcTable = 0;
            std::uint8_t acTable = 0;
        };

        struct Scan
        {
            std::vector<ScanComponent> components;
            std::uint8_t spectralStart = 0;
            std::uint8_t spectralEnd = 0;
            std::uint8_t successiveHigh = 0;
            std::uint8_t successiveLow = 0;
        };

        bool SafeMultiply(std::size_t left, std::size_t right, std::size_t& result)
        {
            if (left != 0 && right > std::numeric_limits<std::size_t>::max() / left)
            {
                return false;
            }
            result = left * right;
            return true;
        }

        std::size_t DivideRoundUp(std::size_t value, std::size_t divisor)
        {
            return value / divisor + (value % divisor != 0 ? 1U : 0U);
        }

        std::uint8_t ClampByte(double value)
        {
            if (value <= 0.0)
            {
                return 0;
            }
            if (value >= 255.0)
            {
                return 255;
            }
            return static_cast<std::uint8_t>(std::lround(value));
        }

        class EntropyReader
        {
        public:
            EntropyReader(const std::uint8_t* data, std::size_t size, std::size_t position)
                : m_Data(data), m_Size(size), m_Position(position)
            {
            }

            bool ReadBit(std::uint32_t& bit, std::string& error)
            {
                if (m_BitsRemaining == 0 && !LoadByte(error))
                {
                    return false;
                }

                --m_BitsRemaining;
                bit = (m_CurrentByte >> m_BitsRemaining) & 1U;
                return true;
            }

            bool ReadBits(std::uint8_t count, std::uint32_t& value, std::string& error)
            {
                value = 0;
                for (std::uint8_t index = 0; index < count; ++index)
                {
                    std::uint32_t bit = 0;
                    if (!ReadBit(bit, error))
                    {
                        return false;
                    }
                    value = (value << 1U) | bit;
                }
                return true;
            }

            bool ConsumeRestart(std::uint8_t expectedIndex, std::string& error)
            {
                m_BitsRemaining = 0;

                std::uint8_t marker = 0;
                if (m_PendingMarker != 0)
                {
                    marker = m_PendingMarker;
                    m_PendingMarker = 0;
                }
                else if (!ReadMarker(marker, error))
                {
                    return false;
                }

                const std::uint8_t expectedMarker = static_cast<std::uint8_t>(0xD0U + expectedIndex);
                if (marker != expectedMarker)
                {
                    error = "JPEG restart marker sequence is invalid";
                    return false;
                }
                return true;
            }

            bool FinishScan(std::uint8_t& marker, std::size_t& nextPosition, std::string& error)
            {
                m_BitsRemaining = 0;
                if (m_PendingMarker != 0)
                {
                    marker = m_PendingMarker;
                    m_PendingMarker = 0;
                    nextPosition = m_Position;
                    return true;
                }

                if (!ReadMarker(marker, error))
                {
                    return false;
                }
                nextPosition = m_Position;
                return true;
            }

        private:
            bool LoadByte(std::string& error)
            {
                if (m_Position >= m_Size)
                {
                    error = "JPEG entropy data is truncated";
                    return false;
                }

                std::uint8_t value = m_Data[m_Position++];
                if (value == 0xFF)
                {
                    while (m_Position < m_Size && m_Data[m_Position] == 0xFF)
                    {
                        ++m_Position;
                    }
                    if (m_Position >= m_Size)
                    {
                        error = "JPEG entropy marker is truncated";
                        return false;
                    }

                    const std::uint8_t following = m_Data[m_Position++];
                    if (following == 0x00)
                    {
                        value = 0xFF;
                    }
                    else
                    {
                        m_PendingMarker = following;
                        error = "JPEG entropy data ended before the scan was complete";
                        return false;
                    }
                }

                m_CurrentByte = value;
                m_BitsRemaining = 8;
                return true;
            }

            bool ReadMarker(std::uint8_t& marker, std::string& error)
            {
                if (m_Position >= m_Size || m_Data[m_Position] != 0xFF)
                {
                    error = "JPEG scan contains unread entropy bytes before its terminating marker";
                    return false;
                }

                while (m_Position < m_Size && m_Data[m_Position] == 0xFF)
                {
                    ++m_Position;
                }
                if (m_Position >= m_Size)
                {
                    error = "JPEG scan terminating marker is truncated";
                    return false;
                }

                marker = m_Data[m_Position++];
                if (marker == 0x00)
                {
                    error = "JPEG scan contains an unexpected stuffed byte after its final coefficient";
                    return false;
                }
                return true;
            }

            const std::uint8_t* m_Data = nullptr;
            std::size_t m_Size = 0;
            std::size_t m_Position = 0;
            std::uint8_t m_CurrentByte = 0;
            std::uint8_t m_BitsRemaining = 0;
            std::uint8_t m_PendingMarker = 0;
        };

        bool HuffmanTable::Decode(EntropyReader& reader, std::uint8_t& symbol, std::string& error) const
        {
            if (!valid)
            {
                error = "JPEG scan references a missing Huffman table";
                return false;
            }

            std::uint32_t code = 0;
            for (std::uint32_t length = 1; length <= 16; ++length)
            {
                std::uint32_t bit = 0;
                if (!reader.ReadBit(bit, error))
                {
                    return false;
                }
                code = (code << 1U) | bit;

                const std::uint32_t first = firstCode[length];
                const std::uint32_t count = counts[length];
                if (code >= first && code - first < count)
                {
                    const std::uint32_t index = firstSymbol[length] + (code - first);
                    if (index >= symbolCount)
                    {
                        error = "JPEG Huffman symbol index is invalid";
                        return false;
                    }
                    symbol = symbols[index];
                    return true;
                }
            }

            error = "JPEG entropy data contains an invalid Huffman code";
            return false;
        }

        bool ReceiveExtend(
            EntropyReader& reader,
            std::uint8_t bitCount,
            std::int32_t& value,
            std::string& error)
        {
            if (bitCount == 0)
            {
                value = 0;
                return true;
            }
            if (bitCount > 16)
            {
                error = "JPEG coefficient magnitude is unsupported";
                return false;
            }

            std::uint32_t bits = 0;
            if (!reader.ReadBits(bitCount, bits, error))
            {
                return false;
            }

            const std::uint32_t threshold = 1U << (bitCount - 1U);
            if (bits < threshold)
            {
                value = static_cast<std::int32_t>(bits) - static_cast<std::int32_t>((1U << bitCount) - 1U);
            }
            else
            {
                value = static_cast<std::int32_t>(bits);
            }
            return true;
        }

        bool ScaleCoefficient(
            std::int32_t value,
            std::uint8_t shift,
            std::int32_t& result,
            std::string& error)
        {
            const std::int64_t scaled = static_cast<std::int64_t>(value) *
                static_cast<std::int64_t>(1U << shift);
            if (scaled < std::numeric_limits<std::int32_t>::min() ||
                scaled > std::numeric_limits<std::int32_t>::max())
            {
                error = "JPEG coefficient exceeds the supported numeric range";
                return false;
            }
            result = static_cast<std::int32_t>(scaled);
            return true;
        }

        class JPEGDecoder
        {
        public:
            JPEGDecoder(const std::uint8_t* data, std::size_t size)
                : m_Data(data), m_Size(size)
            {
            }

            ImageData Decode(std::string& error)
            {
                if (m_Size < 4 || m_Data[0] != 0xFF || m_Data[1] != 0xD8)
                {
                    error = "JPEG data does not begin with an SOI marker";
                    return {};
                }
                m_Position = 2;

                std::uint8_t marker = 0;
                bool markerAlreadyRead = false;
                while (true)
                {
                    if (!markerAlreadyRead && !ReadMarker(marker, error))
                    {
                        return {};
                    }
                    markerAlreadyRead = false;

                    if (marker == 0xD9)
                    {
                        break;
                    }
                    if (marker == 0xD8)
                    {
                        error = "JPEG contains an unexpected nested SOI marker";
                        return {};
                    }
                    if (marker >= 0xD0 && marker <= 0xD7)
                    {
                        error = "JPEG contains a restart marker outside entropy data";
                        return {};
                    }
                    if (marker == 0x01)
                    {
                        continue;
                    }

                    std::size_t segmentEnd = 0;
                    if (!ReadSegmentEnd(segmentEnd, error))
                    {
                        return {};
                    }

                    const bool entropyScan = marker == 0xDA;
                    switch (marker)
                    {
                    case 0xC0:
                        if (!ParseFrame(FrameType::Baseline, segmentEnd, error))
                        {
                            return {};
                        }
                        break;
                    case 0xC2:
                        if (!ParseFrame(FrameType::Progressive, segmentEnd, error))
                        {
                            return {};
                        }
                        break;
                    case 0xC4:
                        if (!ParseHuffmanTables(segmentEnd, error))
                        {
                            return {};
                        }
                        break;
                    case 0xDB:
                        if (!ParseQuantizationTables(segmentEnd, error))
                        {
                            return {};
                        }
                        break;
                    case 0xDD:
                        if (!ParseRestartInterval(segmentEnd, error))
                        {
                            return {};
                        }
                        break;
                    case 0xDA:
                        if (!ParseAndDecodeScan(segmentEnd, marker, error))
                        {
                            return {};
                        }
                        markerAlreadyRead = true;
                        break;
                    case 0xEE:
                        ParseAdobeMarker(segmentEnd);
                        m_Position = segmentEnd;
                        break;
                    case 0xC1:
                    case 0xC3:
                    case 0xC5:
                    case 0xC6:
                    case 0xC7:
                    case 0xC9:
                    case 0xCA:
                    case 0xCB:
                    case 0xCD:
                    case 0xCE:
                    case 0xCF:
                        error = "JPEG frame type is not supported; only baseline and progressive Huffman JPEG are accepted";
                        return {};
                    default:
                        m_Position = segmentEnd;
                        break;
                    }

                    if (!entropyScan && m_Position != segmentEnd)
                    {
                        m_Position = segmentEnd;
                    }
                }

                if (m_FrameType == FrameType::None || !m_SawScan)
                {
                    error = "JPEG does not contain a decodable image frame";
                    return {};
                }
                return Render(error);
            }

        private:
            bool ReadMarker(std::uint8_t& marker, std::string& error)
            {
                while (m_Position < m_Size && m_Data[m_Position] != 0xFF)
                {
                    ++m_Position;
                }
                if (m_Position >= m_Size)
                {
                    error = "JPEG ended before an EOI marker";
                    return false;
                }

                while (m_Position < m_Size && m_Data[m_Position] == 0xFF)
                {
                    ++m_Position;
                }
                if (m_Position >= m_Size)
                {
                    error = "JPEG marker is truncated";
                    return false;
                }

                marker = m_Data[m_Position++];
                if (marker == 0x00)
                {
                    error = "JPEG contains stuffed data outside an entropy scan";
                    return false;
                }
                return true;
            }

            bool ReadSegmentEnd(std::size_t& segmentEnd, std::string& error)
            {
                std::uint16_t length = 0;
                if (!ReadU16(length) || length < 2)
                {
                    error = "JPEG segment length is invalid or truncated";
                    return false;
                }

                const std::size_t payloadLength = static_cast<std::size_t>(length) - 2U;
                if (payloadLength > m_Size - m_Position)
                {
                    error = "JPEG segment extends beyond the input buffer";
                    return false;
                }
                segmentEnd = m_Position + payloadLength;
                return true;
            }

            bool ReadU8(std::uint8_t& value)
            {
                if (m_Position >= m_Size)
                {
                    return false;
                }
                value = m_Data[m_Position++];
                return true;
            }

            bool ReadU16(std::uint16_t& value)
            {
                if (m_Position > m_Size || m_Size - m_Position < 2)
                {
                    return false;
                }
                value = static_cast<std::uint16_t>(
                    (static_cast<std::uint16_t>(m_Data[m_Position]) << 8U) |
                    static_cast<std::uint16_t>(m_Data[m_Position + 1]));
                m_Position += 2;
                return true;
            }

            bool ParseQuantizationTables(std::size_t segmentEnd, std::string& error)
            {
                while (m_Position < segmentEnd)
                {
                    std::uint8_t information = 0;
                    if (!ReadU8(information))
                    {
                        error = "JPEG quantization table header is truncated";
                        return false;
                    }

                    const std::uint8_t precision = information >> 4U;
                    const std::uint8_t tableId = information & 0x0FU;
                    if (precision != 0 || tableId >= m_QuantizationTables.size())
                    {
                        error = "JPEG uses an unsupported quantization table";
                        return false;
                    }
                    if (segmentEnd - m_Position < 64)
                    {
                        error = "JPEG quantization table is truncated";
                        return false;
                    }

                    auto& table = m_QuantizationTables[tableId];
                    for (std::size_t index = 0; index < 64; ++index)
                    {
                        const std::uint8_t value = m_Data[m_Position++];
                        if (value == 0)
                        {
                            error = "JPEG quantization tables may not contain zero values";
                            return false;
                        }
                        table.values[ZigZag[index]] = value;
                    }
                    table.valid = true;
                }
                return m_Position == segmentEnd;
            }

            bool ParseHuffmanTables(std::size_t segmentEnd, std::string& error)
            {
                while (m_Position < segmentEnd)
                {
                    std::uint8_t information = 0;
                    if (!ReadU8(information))
                    {
                        error = "JPEG Huffman table header is truncated";
                        return false;
                    }

                    const std::uint8_t tableClass = information >> 4U;
                    const std::uint8_t tableId = information & 0x0FU;
                    if (tableClass > 1 || tableId >= 4)
                    {
                        error = "JPEG Huffman table identifier is unsupported";
                        return false;
                    }

                    std::array<std::uint8_t, 17> counts{};
                    std::size_t symbolCount = 0;
                    for (std::size_t length = 1; length <= 16; ++length)
                    {
                        if (!ReadU8(counts[length]))
                        {
                            error = "JPEG Huffman code lengths are truncated";
                            return false;
                        }
                        symbolCount += counts[length];
                    }
                    if (symbolCount == 0 || symbolCount > 256 || symbolCount > segmentEnd - m_Position)
                    {
                        error = "JPEG Huffman symbol table is invalid or truncated";
                        return false;
                    }

                    HuffmanTable& table = tableClass == 0 ? m_DCTables[tableId] : m_ACTables[tableId];
                    if (!table.Build(counts, m_Data + m_Position, symbolCount, error))
                    {
                        return false;
                    }
                    m_Position += symbolCount;
                }
                return m_Position == segmentEnd;
            }

            bool ParseFrame(FrameType frameType, std::size_t segmentEnd, std::string& error)
            {
                if (m_FrameType != FrameType::None)
                {
                    error = "JPEG contains multiple image frames";
                    return false;
                }

                std::uint8_t precision = 0;
                std::uint16_t height = 0;
                std::uint16_t width = 0;
                std::uint8_t componentCount = 0;
                if (!ReadU8(precision) || !ReadU16(height) || !ReadU16(width) || !ReadU8(componentCount))
                {
                    error = "JPEG frame header is truncated";
                    return false;
                }
                if (precision != 8)
                {
                    error = "JPEG sample precision is unsupported; only 8-bit images are accepted";
                    return false;
                }
                if (width == 0 || height == 0 || (componentCount != 1 && componentCount != 3))
                {
                    error = "JPEG dimensions or component count are unsupported";
                    return false;
                }
                if (segmentEnd - m_Position != static_cast<std::size_t>(componentCount) * 3U)
                {
                    error = "JPEG frame component data has an invalid length";
                    return false;
                }

                m_Width = width;
                m_Height = height;
                m_FrameType = frameType;
                m_Components.clear();
                m_Components.resize(componentCount);
                m_MaxHorizontalSampling = 0;
                m_MaxVerticalSampling = 0;

                for (std::size_t index = 0; index < componentCount; ++index)
                {
                    Component& component = m_Components[index];
                    std::uint8_t sampling = 0;
                    if (!ReadU8(component.id) || !ReadU8(sampling) || !ReadU8(component.quantizationTable))
                    {
                        error = "JPEG frame component data is truncated";
                        return false;
                    }
                    component.horizontalSampling = sampling >> 4U;
                    component.verticalSampling = sampling & 0x0FU;
                    if (component.horizontalSampling == 0 || component.horizontalSampling > 4 ||
                        component.verticalSampling == 0 || component.verticalSampling > 4 ||
                        component.quantizationTable >= m_QuantizationTables.size())
                    {
                        error = "JPEG frame component sampling or quantization identifier is unsupported";
                        return false;
                    }
                    for (std::size_t previous = 0; previous < index; ++previous)
                    {
                        if (m_Components[previous].id == component.id)
                        {
                            error = "JPEG frame contains duplicate component identifiers";
                            return false;
                        }
                    }

                    m_MaxHorizontalSampling = std::max(m_MaxHorizontalSampling, component.horizontalSampling);
                    m_MaxVerticalSampling = std::max(m_MaxVerticalSampling, component.verticalSampling);
                }

                const std::size_t mcuWidth = static_cast<std::size_t>(m_MaxHorizontalSampling) * 8U;
                const std::size_t mcuHeight = static_cast<std::size_t>(m_MaxVerticalSampling) * 8U;
                m_MCUColumns = DivideRoundUp(m_Width, mcuWidth);
                m_MCURows = DivideRoundUp(m_Height, mcuHeight);

                std::size_t totalCoefficientBytes = 0;
                for (Component& component : m_Components)
                {
                    const std::size_t scaledWidthNumerator = static_cast<std::size_t>(m_Width) * component.horizontalSampling;
                    const std::size_t scaledHeightNumerator = static_cast<std::size_t>(m_Height) * component.verticalSampling;
                    const std::size_t componentWidth = DivideRoundUp(scaledWidthNumerator, m_MaxHorizontalSampling);
                    const std::size_t componentHeight = DivideRoundUp(scaledHeightNumerator, m_MaxVerticalSampling);
                    component.actualBlockColumns = DivideRoundUp(componentWidth, 8U);
                    component.actualBlockRows = DivideRoundUp(componentHeight, 8U);
                    component.paddedBlockColumns = m_MCUColumns * component.horizontalSampling;
                    component.paddedBlockRows = m_MCURows * component.verticalSampling;

                    std::size_t blockCount = 0;
                    std::size_t coefficientCount = 0;
                    std::size_t coefficientBytes = 0;
                    if (!SafeMultiply(component.paddedBlockColumns, component.paddedBlockRows, blockCount) ||
                        !SafeMultiply(blockCount, 64U, coefficientCount) ||
                        !SafeMultiply(coefficientCount, sizeof(std::int32_t), coefficientBytes) ||
                        coefficientBytes > MaxCoefficientBytes - totalCoefficientBytes)
                    {
                        error = "JPEG coefficient storage exceeds the decoder limit";
                        return false;
                    }
                    totalCoefficientBytes += coefficientBytes;
                    component.coefficients.assign(coefficientCount, 0);
                }
                return true;
            }

            bool ParseRestartInterval(std::size_t segmentEnd, std::string& error)
            {
                if (segmentEnd - m_Position != 2 || !ReadU16(m_RestartInterval))
                {
                    error = "JPEG restart interval segment is invalid";
                    return false;
                }
                return true;
            }

            void ParseAdobeMarker(std::size_t segmentEnd)
            {
                if (segmentEnd - m_Position >= 12 &&
                    m_Data[m_Position] == 'A' && m_Data[m_Position + 1] == 'd' &&
                    m_Data[m_Position + 2] == 'o' && m_Data[m_Position + 3] == 'b' &&
                    m_Data[m_Position + 4] == 'e')
                {
                    m_AdobeTransform = m_Data[m_Position + 11];
                }
            }

            bool ParseAndDecodeScan(std::size_t segmentEnd, std::uint8_t& nextMarker, std::string& error)
            {
                if (m_FrameType == FrameType::None)
                {
                    error = "JPEG scan appears before the frame header";
                    return false;
                }

                std::uint8_t componentCount = 0;
                if (!ReadU8(componentCount) || componentCount == 0 || componentCount > m_Components.size())
                {
                    error = "JPEG scan component count is invalid";
                    return false;
                }

                Scan scan;
                scan.components.reserve(componentCount);
                for (std::uint8_t index = 0; index < componentCount; ++index)
                {
                    std::uint8_t componentId = 0;
                    std::uint8_t selectors = 0;
                    if (!ReadU8(componentId) || !ReadU8(selectors))
                    {
                        error = "JPEG scan component selectors are truncated";
                        return false;
                    }

                    const auto component = std::find_if(
                        m_Components.begin(),
                        m_Components.end(),
                        [componentId](const Component& candidate) { return candidate.id == componentId; });
                    if (component == m_Components.end())
                    {
                        error = "JPEG scan references an unknown component";
                        return false;
                    }
                    const std::size_t componentIndex = static_cast<std::size_t>(component - m_Components.begin());
                    for (const ScanComponent& existing : scan.components)
                    {
                        if (existing.componentIndex == componentIndex)
                        {
                            error = "JPEG scan contains a duplicate component";
                            return false;
                        }
                    }

                    ScanComponent scanComponent;
                    scanComponent.componentIndex = componentIndex;
                    scanComponent.dcTable = selectors >> 4U;
                    scanComponent.acTable = selectors & 0x0FU;
                    if (scanComponent.dcTable >= 4 || scanComponent.acTable >= 4)
                    {
                        error = "JPEG scan references an unsupported Huffman table identifier";
                        return false;
                    }
                    scan.components.push_back(scanComponent);
                }

                std::uint8_t approximation = 0;
                if (!ReadU8(scan.spectralStart) || !ReadU8(scan.spectralEnd) || !ReadU8(approximation))
                {
                    error = "JPEG scan parameters are truncated";
                    return false;
                }
                scan.successiveHigh = approximation >> 4U;
                scan.successiveLow = approximation & 0x0FU;
                if (m_Position != segmentEnd)
                {
                    error = "JPEG scan header length is invalid";
                    return false;
                }

                if (!ValidateScan(scan, error))
                {
                    return false;
                }

                EntropyReader reader(m_Data, m_Size, m_Position);
                if (!DecodeScan(scan, reader, error))
                {
                    return false;
                }

                if (!reader.FinishScan(nextMarker, m_Position, error))
                {
                    return false;
                }
                m_SawScan = true;
                return true;
            }

            bool ValidateScan(const Scan& scan, std::string& error) const
            {
                if (m_FrameType == FrameType::Baseline)
                {
                    if (scan.spectralStart != 0 || scan.spectralEnd != 63 ||
                        scan.successiveHigh != 0 || scan.successiveLow != 0)
                    {
                        error = "Baseline JPEG scan parameters are invalid";
                        return false;
                    }
                    for (const ScanComponent& component : scan.components)
                    {
                        if (!m_DCTables[component.dcTable].valid || !m_ACTables[component.acTable].valid)
                        {
                            error = "Baseline JPEG scan references a missing Huffman table";
                            return false;
                        }
                    }
                    return true;
                }

                if (scan.spectralStart > scan.spectralEnd || scan.spectralEnd > 63 ||
                    scan.successiveHigh > 13 || scan.successiveLow > 13)
                {
                    error = "Progressive JPEG scan parameters are invalid";
                    return false;
                }
                if (scan.spectralStart == 0)
                {
                    if (scan.spectralEnd != 0)
                    {
                        error = "Progressive JPEG DC scan has an invalid spectral range";
                        return false;
                    }
                    if (scan.successiveHigh == 0)
                    {
                        for (const ScanComponent& component : scan.components)
                        {
                            if (!m_DCTables[component.dcTable].valid)
                            {
                                error = "Progressive JPEG DC scan references a missing Huffman table";
                                return false;
                            }
                        }
                    }
                }
                else
                {
                    if (scan.components.size() != 1)
                    {
                        error = "Progressive JPEG AC scans must contain exactly one component";
                        return false;
                    }
                    if (!m_ACTables[scan.components[0].acTable].valid)
                    {
                        error = "Progressive JPEG AC scan references a missing Huffman table";
                        return false;
                    }
                }
                if (scan.successiveHigh != 0 && scan.successiveHigh != scan.successiveLow + 1U)
                {
                    error = "Progressive JPEG successive approximation is invalid";
                    return false;
                }
                return true;
            }

            bool DecodeScan(const Scan& scan, EntropyReader& reader, std::string& error)
            {
                std::array<std::int32_t, 4> dcPredictors{};
                std::uint32_t eobRun = 0;
                std::uint8_t expectedRestart = 0;

                const bool interleaved = scan.components.size() > 1;
                std::size_t unitColumns = 0;
                std::size_t unitRows = 0;
                if (interleaved)
                {
                    unitColumns = m_MCUColumns;
                    unitRows = m_MCURows;
                }
                else
                {
                    const Component& component = m_Components[scan.components[0].componentIndex];
                    unitColumns = component.actualBlockColumns;
                    unitRows = component.actualBlockRows;
                }

                std::size_t unitIndex = 0;
                for (std::size_t row = 0; row < unitRows; ++row)
                {
                    for (std::size_t column = 0; column < unitColumns; ++column, ++unitIndex)
                    {
                        if (m_RestartInterval != 0 && unitIndex != 0 && unitIndex % m_RestartInterval == 0)
                        {
                            if (!reader.ConsumeRestart(expectedRestart, error))
                            {
                                return false;
                            }
                            expectedRestart = static_cast<std::uint8_t>((expectedRestart + 1U) & 7U);
                            dcPredictors.fill(0);
                            eobRun = 0;
                        }

                        if (interleaved)
                        {
                            for (const ScanComponent& scanComponent : scan.components)
                            {
                                Component& component = m_Components[scanComponent.componentIndex];
                                for (std::size_t vertical = 0; vertical < component.verticalSampling; ++vertical)
                                {
                                    for (std::size_t horizontal = 0; horizontal < component.horizontalSampling; ++horizontal)
                                    {
                                        const std::size_t blockColumn = column * component.horizontalSampling + horizontal;
                                        const std::size_t blockRow = row * component.verticalSampling + vertical;
                                        if (!DecodeBlock(
                                                scan,
                                                scanComponent,
                                                component,
                                                blockColumn,
                                                blockRow,
                                                dcPredictors[scanComponent.componentIndex],
                                                eobRun,
                                                reader,
                                                error))
                                        {
                                            return false;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            const ScanComponent& scanComponent = scan.components[0];
                            Component& component = m_Components[scanComponent.componentIndex];
                            if (!DecodeBlock(
                                    scan,
                                    scanComponent,
                                    component,
                                    column,
                                    row,
                                    dcPredictors[scanComponent.componentIndex],
                                    eobRun,
                                    reader,
                                    error))
                            {
                                return false;
                            }
                        }
                    }
                }
                return true;
            }

            bool DecodeBlock(
                const Scan& scan,
                const ScanComponent& scanComponent,
                Component& component,
                std::size_t blockColumn,
                std::size_t blockRow,
                std::int32_t& dcPredictor,
                std::uint32_t& eobRun,
                EntropyReader& reader,
                std::string& error)
            {
                if (blockColumn >= component.paddedBlockColumns || blockRow >= component.paddedBlockRows)
                {
                    error = "JPEG scan addresses a block outside the component bounds";
                    return false;
                }
                std::int32_t* block = component.coefficients.data() +
                    (blockRow * component.paddedBlockColumns + blockColumn) * 64U;

                if (m_FrameType == FrameType::Baseline)
                {
                    return DecodeBaselineBlock(scanComponent, block, dcPredictor, reader, error);
                }
                if (scan.spectralStart == 0)
                {
                    return scan.successiveHigh == 0
                        ? DecodeProgressiveDCFirst(scan, scanComponent, block, dcPredictor, reader, error)
                        : DecodeProgressiveDCRefinement(scan, block, reader, error);
                }
                return scan.successiveHigh == 0
                    ? DecodeProgressiveACFirst(scan, scanComponent, block, eobRun, reader, error)
                    : DecodeProgressiveACRefinement(scan, scanComponent, block, eobRun, reader, error);
            }

            bool DecodeBaselineBlock(
                const ScanComponent& component,
                std::int32_t* block,
                std::int32_t& dcPredictor,
                EntropyReader& reader,
                std::string& error) const
            {
                std::uint8_t dcSize = 0;
                if (!m_DCTables[component.dcTable].Decode(reader, dcSize, error))
                {
                    return false;
                }
                if (dcSize > 11)
                {
                    error = "JPEG DC coefficient magnitude is invalid";
                    return false;
                }

                std::int32_t difference = 0;
                if (!ReceiveExtend(reader, dcSize, difference, error))
                {
                    return false;
                }
                const std::int64_t predicted = static_cast<std::int64_t>(dcPredictor) + difference;
                if (predicted < std::numeric_limits<std::int32_t>::min() ||
                    predicted > std::numeric_limits<std::int32_t>::max())
                {
                    error = "JPEG DC predictor exceeds the supported numeric range";
                    return false;
                }
                dcPredictor = static_cast<std::int32_t>(predicted);
                block[0] = dcPredictor;

                std::uint8_t coefficient = 1;
                while (coefficient <= 63)
                {
                    std::uint8_t symbol = 0;
                    if (!m_ACTables[component.acTable].Decode(reader, symbol, error))
                    {
                        return false;
                    }
                    const std::uint8_t run = symbol >> 4U;
                    const std::uint8_t size = symbol & 0x0FU;
                    if (size == 0)
                    {
                        if (run == 0)
                        {
                            break;
                        }
                        if (run != 15 || coefficient > 48)
                        {
                            error = "JPEG AC zero run exceeds the block";
                            return false;
                        }
                        coefficient = static_cast<std::uint8_t>(coefficient + 16U);
                        continue;
                    }
                    if (size > 10 || static_cast<unsigned>(coefficient) + run > 63U)
                    {
                        error = "JPEG AC coefficient is invalid";
                        return false;
                    }
                    coefficient = static_cast<std::uint8_t>(coefficient + run);
                    std::int32_t value = 0;
                    if (!ReceiveExtend(reader, size, value, error))
                    {
                        return false;
                    }
                    block[ZigZag[coefficient]] = value;
                    ++coefficient;
                }
                return true;
            }

            bool DecodeProgressiveDCFirst(
                const Scan& scan,
                const ScanComponent& component,
                std::int32_t* block,
                std::int32_t& dcPredictor,
                EntropyReader& reader,
                std::string& error) const
            {
                std::uint8_t dcSize = 0;
                if (!m_DCTables[component.dcTable].Decode(reader, dcSize, error))
                {
                    return false;
                }
                if (dcSize > 11)
                {
                    error = "Progressive JPEG DC coefficient magnitude is invalid";
                    return false;
                }

                std::int32_t difference = 0;
                if (!ReceiveExtend(reader, dcSize, difference, error))
                {
                    return false;
                }
                const std::int64_t predicted = static_cast<std::int64_t>(dcPredictor) + difference;
                if (predicted < std::numeric_limits<std::int32_t>::min() ||
                    predicted > std::numeric_limits<std::int32_t>::max())
                {
                    error = "Progressive JPEG DC predictor exceeds the supported numeric range";
                    return false;
                }
                dcPredictor = static_cast<std::int32_t>(predicted);
                return ScaleCoefficient(dcPredictor, scan.successiveLow, block[0], error);
            }

            bool DecodeProgressiveDCRefinement(
                const Scan& scan,
                std::int32_t* block,
                EntropyReader& reader,
                std::string& error) const
            {
                std::uint32_t bit = 0;
                if (!reader.ReadBit(bit, error))
                {
                    return false;
                }
                if (bit != 0)
                {
                    RefineCoefficient(block[0], 1 << scan.successiveLow);
                }
                return true;
            }

            bool DecodeProgressiveACFirst(
                const Scan& scan,
                const ScanComponent& component,
                std::int32_t* block,
                std::uint32_t& eobRun,
                EntropyReader& reader,
                std::string& error) const
            {
                if (eobRun != 0)
                {
                    --eobRun;
                    return true;
                }

                std::uint8_t coefficient = scan.spectralStart;
                while (coefficient <= scan.spectralEnd)
                {
                    std::uint8_t symbol = 0;
                    if (!m_ACTables[component.acTable].Decode(reader, symbol, error))
                    {
                        return false;
                    }
                    const std::uint8_t run = symbol >> 4U;
                    const std::uint8_t size = symbol & 0x0FU;
                    if (size == 0)
                    {
                        if (run == 15)
                        {
                            if (static_cast<unsigned>(coefficient) + 15U > scan.spectralEnd)
                            {
                                error = "Progressive JPEG AC zero run exceeds the spectral band";
                                return false;
                            }
                            coefficient = static_cast<std::uint8_t>(coefficient + 16U);
                            continue;
                        }

                        std::uint32_t extra = 0;
                        if (run != 0 && !reader.ReadBits(run, extra, error))
                        {
                            return false;
                        }
                        eobRun = (1U << run) + extra;
                        --eobRun;
                        break;
                    }
                    if (size > 10 || static_cast<unsigned>(coefficient) + run > scan.spectralEnd)
                    {
                        error = "Progressive JPEG AC coefficient is invalid";
                        return false;
                    }
                    coefficient = static_cast<std::uint8_t>(coefficient + run);
                    std::int32_t value = 0;
                    if (!ReceiveExtend(reader, size, value, error))
                    {
                        return false;
                    }
                    if (!ScaleCoefficient(
                            value,
                            scan.successiveLow,
                            block[ZigZag[coefficient]],
                            error))
                    {
                        return false;
                    }
                    ++coefficient;
                }
                return true;
            }

            bool DecodeProgressiveACRefinement(
                const Scan& scan,
                const ScanComponent& component,
                std::int32_t* block,
                std::uint32_t& eobRun,
                EntropyReader& reader,
                std::string& error) const
            {
                const std::int32_t refinement = 1 << scan.successiveLow;
                std::uint8_t coefficient = scan.spectralStart;

                if (eobRun == 0)
                {
                    while (coefficient <= scan.spectralEnd)
                    {
                        std::uint8_t symbol = 0;
                        if (!m_ACTables[component.acTable].Decode(reader, symbol, error))
                        {
                            return false;
                        }

                        int run = symbol >> 4U;
                        const std::uint8_t size = symbol & 0x0FU;
                        std::int32_t newCoefficient = 0;
                        bool zeroRunLength = false;

                        if (size != 0)
                        {
                            if (size != 1)
                            {
                                error = "Progressive JPEG refinement coefficient has an invalid size";
                                return false;
                            }
                            std::uint32_t sign = 0;
                            if (!reader.ReadBit(sign, error))
                            {
                                return false;
                            }
                            newCoefficient = sign != 0 ? refinement : -refinement;
                        }
                        else if (run != 15)
                        {
                            std::uint32_t extra = 0;
                            if (run != 0 && !reader.ReadBits(static_cast<std::uint8_t>(run), extra, error))
                            {
                                return false;
                            }
                            eobRun = (1U << run) + extra;
                            break;
                        }
                        else
                        {
                            zeroRunLength = true;
                        }

                        while (coefficient <= scan.spectralEnd)
                        {
                            std::int32_t& current = block[ZigZag[coefficient]];
                            if (current != 0)
                            {
                                std::uint32_t bit = 0;
                                if (!reader.ReadBit(bit, error))
                                {
                                    return false;
                                }
                                if (bit != 0)
                                {
                                    RefineCoefficient(current, refinement);
                                }
                            }
                            else
                            {
                                if (run == 0)
                                {
                                    break;
                                }
                                --run;
                            }
                            ++coefficient;
                        }

                        if (newCoefficient != 0)
                        {
                            if (coefficient > scan.spectralEnd)
                            {
                                error = "Progressive JPEG refinement run exceeds the spectral band";
                                return false;
                            }
                            block[ZigZag[coefficient]] = newCoefficient;
                            ++coefficient;
                        }
                        else if (zeroRunLength)
                        {
                            if (coefficient > scan.spectralEnd)
                            {
                                error = "Progressive JPEG refinement zero run exceeds the spectral band";
                                return false;
                            }
                            ++coefficient;
                        }
                    }
                }

                if (eobRun != 0)
                {
                    while (coefficient <= scan.spectralEnd)
                    {
                        std::int32_t& current = block[ZigZag[coefficient]];
                        if (current != 0)
                        {
                            std::uint32_t bit = 0;
                            if (!reader.ReadBit(bit, error))
                            {
                                return false;
                            }
                            if (bit != 0)
                            {
                                RefineCoefficient(current, refinement);
                            }
                        }
                        ++coefficient;
                    }
                    --eobRun;
                }
                return true;
            }

            static void RefineCoefficient(std::int32_t& coefficient, std::int32_t refinement)
            {
                if ((std::abs(coefficient) & refinement) == 0)
                {
                    coefficient += coefficient >= 0 ? refinement : -refinement;
                }
            }

            ImageData Render(std::string& error)
            {
                std::size_t totalSampleBytes = 0;
                for (Component& component : m_Components)
                {
                    if (!m_QuantizationTables[component.quantizationTable].valid)
                    {
                        error = "JPEG component references a missing quantization table";
                        return {};
                    }

                    std::size_t sampleWidth = component.paddedBlockColumns * 8U;
                    std::size_t sampleHeight = component.paddedBlockRows * 8U;
                    std::size_t sampleCount = 0;
                    if (!SafeMultiply(sampleWidth, sampleHeight, sampleCount) ||
                        sampleCount > MaxDecodedImageBytes - totalSampleBytes)
                    {
                        error = "JPEG component sample storage exceeds the decoder limit";
                        return {};
                    }
                    totalSampleBytes += sampleCount;
                    component.samples.assign(sampleCount, 0);

                    for (std::size_t blockRow = 0; blockRow < component.paddedBlockRows; ++blockRow)
                    {
                        for (std::size_t blockColumn = 0; blockColumn < component.paddedBlockColumns; ++blockColumn)
                        {
                            const std::int32_t* block = component.coefficients.data() +
                                (blockRow * component.paddedBlockColumns + blockColumn) * 64U;
                            InverseDCT(
                                block,
                                m_QuantizationTables[component.quantizationTable].values,
                                component.samples.data(),
                                sampleWidth,
                                blockColumn * 8U,
                                blockRow * 8U);
                        }
                    }
                }

                std::size_t pixelCount = 0;
                std::size_t byteCount = 0;
                if (!SafeMultiply(m_Width, m_Height, pixelCount) ||
                    !SafeMultiply(pixelCount, 3U, byteCount) ||
                    byteCount > MaxDecodedImageBytes ||
                    m_Width > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
                    m_Height > static_cast<std::size_t>(std::numeric_limits<int>::max()))
                {
                    error = "JPEG decoded image exceeds the output limit";
                    return {};
                }

                auto* output = new (std::nothrow) unsigned char[byteCount];
                if (!output)
                {
                    error = "Failed to allocate memory for decoded JPEG pixels";
                    return {};
                }

                const bool directRGB = IsDirectRGB();
                for (std::size_t y = 0; y < m_Height; ++y)
                {
                    for (std::size_t x = 0; x < m_Width; ++x)
                    {
                        const std::size_t outputIndex = (y * m_Width + x) * 3U;
                        if (m_Components.size() == 1)
                        {
                            const std::uint8_t gray = SampleComponent(m_Components[0], x, y);
                            output[outputIndex] = gray;
                            output[outputIndex + 1] = gray;
                            output[outputIndex + 2] = gray;
                        }
                        else if (directRGB)
                        {
                            output[outputIndex] = SampleComponent(m_Components[0], x, y);
                            output[outputIndex + 1] = SampleComponent(m_Components[1], x, y);
                            output[outputIndex + 2] = SampleComponent(m_Components[2], x, y);
                        }
                        else
                        {
                            const double luminance = SampleComponent(m_Components[0], x, y);
                            const double blueDifference = static_cast<double>(SampleComponent(m_Components[1], x, y)) - 128.0;
                            const double redDifference = static_cast<double>(SampleComponent(m_Components[2], x, y)) - 128.0;
                            output[outputIndex] = ClampByte(luminance + 1.402 * redDifference);
                            output[outputIndex + 1] = ClampByte(
                                luminance - 0.344136 * blueDifference - 0.714136 * redDifference);
                            output[outputIndex + 2] = ClampByte(luminance + 1.772 * blueDifference);
                        }
                    }
                }

                ImageData result;
                result.Data = output;
                result.Width = static_cast<int>(m_Width);
                result.Height = static_cast<int>(m_Height);
                result.Channels = 3;
                return result;
            }

            bool IsDirectRGB() const
            {
                if (m_AdobeTransform == 0)
                {
                    return true;
                }
                return m_Components.size() == 3 &&
                    m_Components[0].id == static_cast<std::uint8_t>('R') &&
                    m_Components[1].id == static_cast<std::uint8_t>('G') &&
                    m_Components[2].id == static_cast<std::uint8_t>('B');
            }

            std::uint8_t SampleComponent(const Component& component, std::size_t x, std::size_t y) const
            {
                const std::size_t sampleWidth = component.paddedBlockColumns * 8U;
                const std::size_t sampleHeight = component.paddedBlockRows * 8U;
                const std::size_t sampleX = std::min(
                    x * component.horizontalSampling / m_MaxHorizontalSampling,
                    sampleWidth - 1U);
                const std::size_t sampleY = std::min(
                    y * component.verticalSampling / m_MaxVerticalSampling,
                    sampleHeight - 1U);
                return component.samples[sampleY * sampleWidth + sampleX];
            }

            static void InverseDCT(
                const std::int32_t* coefficients,
                const std::array<std::uint16_t, 64>& quantization,
                std::uint8_t* destination,
                std::size_t destinationStride,
                std::size_t destinationX,
                std::size_t destinationY)
            {
                bool hasAC = false;
                for (std::size_t index = 1; index < 64; ++index)
                {
                    if (coefficients[index] != 0)
                    {
                        hasAC = true;
                        break;
                    }
                }
                if (!hasAC)
                {
                    const std::uint8_t value = ClampByte(
                        static_cast<double>(coefficients[0]) * quantization[0] / 8.0 + 128.0);
                    for (std::size_t y = 0; y < 8; ++y)
                    {
                        std::fill_n(
                            destination + (destinationY + y) * destinationStride + destinationX,
                            8,
                            value);
                    }
                    return;
                }

                static const std::array<std::array<double, 8>, 8> basis = [] {
                    std::array<std::array<double, 8>, 8> result{};
                    for (std::size_t position = 0; position < 8; ++position)
                    {
                        for (std::size_t frequency = 0; frequency < 8; ++frequency)
                        {
                            const double normalization = frequency == 0 ? 1.0 / std::sqrt(2.0) : 1.0;
                            result[position][frequency] = normalization * std::cos(
                                (static_cast<double>(2 * position + 1) * frequency * Pi) / 16.0);
                        }
                    }
                    return result;
                }();

                double temporary[8][8] = {};
                for (std::size_t frequencyY = 0; frequencyY < 8; ++frequencyY)
                {
                    for (std::size_t x = 0; x < 8; ++x)
                    {
                        double sum = 0.0;
                        for (std::size_t frequencyX = 0; frequencyX < 8; ++frequencyX)
                        {
                            const std::size_t index = frequencyY * 8U + frequencyX;
                            sum += basis[x][frequencyX] *
                                static_cast<double>(coefficients[index]) * quantization[index];
                        }
                        temporary[frequencyY][x] = sum;
                    }
                }

                for (std::size_t y = 0; y < 8; ++y)
                {
                    for (std::size_t x = 0; x < 8; ++x)
                    {
                        double sum = 0.0;
                        for (std::size_t frequencyY = 0; frequencyY < 8; ++frequencyY)
                        {
                            sum += basis[y][frequencyY] * temporary[frequencyY][x];
                        }
                        destination[(destinationY + y) * destinationStride + destinationX + x] =
                            ClampByte(sum * 0.25 + 128.0);
                    }
                }
            }

            const std::uint8_t* m_Data = nullptr;
            std::size_t m_Size = 0;
            std::size_t m_Position = 0;
            std::size_t m_Width = 0;
            std::size_t m_Height = 0;
            std::size_t m_MCUColumns = 0;
            std::size_t m_MCURows = 0;
            std::uint8_t m_MaxHorizontalSampling = 0;
            std::uint8_t m_MaxVerticalSampling = 0;
            std::uint16_t m_RestartInterval = 0;
            int m_AdobeTransform = -1;
            FrameType m_FrameType = FrameType::None;
            bool m_SawScan = false;
            std::array<QuantizationTable, 4> m_QuantizationTables{};
            std::array<HuffmanTable, 4> m_DCTables{};
            std::array<HuffmanTable, 4> m_ACTables{};
            std::vector<Component> m_Components;
        };
    } // namespace

    thread_local std::string JPEGLoader::s_LastError;

    ImageData JPEGLoader::LoadFromFile(const std::string& filepath)
    {
        s_LastError.clear();

        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file)
        {
            SetError("Failed to open JPEG file: " + filepath);
            return {};
        }

        const std::streampos end = file.tellg();
        if (end <= 0)
        {
            SetError("JPEG file is empty: " + filepath);
            return {};
        }

        const auto fileSize = static_cast<std::size_t>(end);
        if (fileSize > MaxInputBytes)
        {
            SetError("JPEG file is too large to decode: " + filepath);
            return {};
        }

        try
        {
            std::vector<std::uint8_t> bytes(fileSize);
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
            if (!file)
            {
                SetError("Failed to read JPEG file: " + filepath);
                return {};
            }

            return Decode(bytes.data(), bytes.size());
        }
        catch (const std::bad_alloc&)
        {
            SetError("Failed to allocate memory for JPEG file data: " + filepath);
            return {};
        }
        catch (const std::length_error&)
        {
            SetError("JPEG file size exceeds the decoder container limit: " + filepath);
            return {};
        }
    }

    ImageData JPEGLoader::LoadFromMemory(const std::uint8_t* data, std::size_t size)
    {
        s_LastError.clear();
        return Decode(data, size);
    }

    const std::string& JPEGLoader::GetLastError()
    {
        return s_LastError;
    }

    ImageData JPEGLoader::Decode(const std::uint8_t* data, std::size_t size)
    {
        if (!data || size == 0)
        {
            SetError("Invalid or empty JPEG data");
            return {};
        }
        if (size > MaxInputBytes)
        {
            SetError("JPEG memory buffer is too large to decode");
            return {};
        }

        try
        {
            std::string error;
            JPEGDecoder decoder(data, size);
            ImageData result = decoder.Decode(error);
            if (!result.Data)
            {
                SetError(error.empty() ? "JPEG decoding failed" : error);
            }
            return result;
        }
        catch (const std::bad_alloc&)
        {
            SetError("Failed to allocate memory while decoding JPEG data");
            return {};
        }
        catch (const std::length_error&)
        {
            SetError("JPEG dimensions exceed the decoder container limit");
            return {};
        }
    }

    void JPEGLoader::SetError(const std::string& error)
    {
        s_LastError = error;
    }
} // namespace Pyramid::Util
