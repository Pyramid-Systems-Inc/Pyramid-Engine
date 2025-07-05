#include "Pyramid/Util/JPEGDequantizer.hpp"
#include "Pyramid/Util/Log.hpp"
#include <algorithm>

namespace Pyramid
{
    namespace Util
    {
        // JPEG zigzag order for 8x8 DCT blocks (same as in JPEGCoefficientDecoder)
        const int JPEGDequantizer::ZIGZAG_ORDER[64] = {
             0,  1,  8, 16,  9,  2,  3, 10,
            17, 24, 32, 25, 18, 11,  4,  5,
            12, 19, 26, 33, 40, 48, 41, 34,
            27, 20, 13,  6,  7, 14, 21, 28,
            35, 42, 49, 56, 57, 50, 43, 36,
            29, 22, 15, 23, 30, 37, 44, 51,
            58, 59, 52, 45, 38, 31, 39, 46,
            53, 60, 61, 54, 47, 55, 62, 63
        };

        JPEGDequantizer::JPEGDequantizer()
        {
            Clear();
        }

        bool JPEGDequantizer::SetQuantizationTable(int tableId, const uint16_t* values, int precision)
        {
            if (tableId < 0 || tableId > 3)
            {
                SetError("Invalid quantization table ID");
                return false;
            }

            if (!values)
            {
                SetError("Null quantization values pointer");
                return false;
            }

            if (precision < 0 || precision > 1)
            {
                SetError("Invalid quantization table precision");
                return false;
            }

            // Copy the quantization values
            QuantizationTable& table = m_Tables[tableId];
            table.defined = true;
            table.precision = precision;
            
            for (int i = 0; i < 64; ++i)
            {
                table.values[i] = values[i];
                
                // Validate quantization values (must be non-zero)
                if (table.values[i] == 0)
                {
                    SetError("Invalid quantization value (zero)");
                    table.defined = false;
                    return false;
                }
            }

            return true;
        }

        bool JPEGDequantizer::DequantizeBlock(int16_t coefficients[64], int tableId)
        {
            if (tableId < 0 || tableId > 3)
            {
                SetError("Invalid quantization table ID");
                return false;
            }

            if (!coefficients)
            {
                SetError("Null coefficients pointer");
                return false;
            }

            if (!m_Tables[tableId].defined)
            {
                SetError("Quantization table not defined");
                return false;
            }

            const QuantizationTable& table = m_Tables[tableId];

            // Dequantize each coefficient
            // coefficients[i] = quantized_coefficient * quantization_table[i]
            for (int i = 0; i < 64; ++i)
            {
                // Multiply the coefficient by the quantization value
                int32_t dequantized = static_cast<int32_t>(coefficients[i]) * static_cast<int32_t>(table.values[i]);
                
                // Clamp to int16_t range to prevent overflow
                if (dequantized > INT16_MAX)
                {
                    coefficients[i] = INT16_MAX;
                }
                else if (dequantized < INT16_MIN)
                {
                    coefficients[i] = INT16_MIN;
                }
                else
                {
                    coefficients[i] = static_cast<int16_t>(dequantized);
                }
            }

            return true;
        }

        bool JPEGDequantizer::IsTableDefined(int tableId) const
        {
            if (tableId < 0 || tableId > 3)
            {
                return false;
            }
            
            return m_Tables[tableId].defined;
        }

        const std::string& JPEGDequantizer::GetLastError() const
        {
            return m_LastError;
        }

        void JPEGDequantizer::Clear()
        {
            for (int i = 0; i < 4; ++i)
            {
                m_Tables[i].defined = false;
                m_Tables[i].precision = 0;
                std::fill(m_Tables[i].values, m_Tables[i].values + 64, 1);
            }
            m_LastError.clear();
        }

        void JPEGDequantizer::SetError(const std::string& error)
        {
            m_LastError = error;
            PYRAMID_LOG_ERROR("JPEGDequantizer: ", error);
        }

    } // namespace Util
} // namespace Pyramid
