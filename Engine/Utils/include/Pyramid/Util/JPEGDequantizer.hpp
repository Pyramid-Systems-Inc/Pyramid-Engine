#pragma once

#include <cstdint>
#include <string>

namespace Pyramid
{
    namespace Util
    {
        /**
         * @brief JPEG dequantization implementation
         * 
         * This class handles the dequantization of JPEG DCT coefficients using
         * quantization tables defined in DQT segments. Dequantization reverses
         * the quantization step by multiplying each coefficient by its corresponding
         * quantization table value.
         */
        class JPEGDequantizer
        {
        public:
            /**
             * @brief Construct a JPEG dequantizer
             */
            JPEGDequantizer();

            /**
             * @brief Set a quantization table
             * 
             * @param tableId Quantization table ID (0-3)
             * @param values Quantization values in zigzag order (64 values)
             * @param precision Table precision (0 = 8-bit, 1 = 16-bit)
             * @return true if successful, false otherwise
             */
            bool SetQuantizationTable(int tableId, const uint16_t* values, int precision);

            /**
             * @brief Dequantize a block of DCT coefficients
             * 
             * @param coefficients Input/output array of 64 coefficients (in zigzag order)
             * @param tableId Quantization table ID to use (0-3)
             * @return true if successful, false otherwise
             */
            bool DequantizeBlock(int16_t coefficients[64], int tableId);

            /**
             * @brief Check if a quantization table is defined
             * 
             * @param tableId Table ID to check (0-3)
             * @return true if table is defined, false otherwise
             */
            bool IsTableDefined(int tableId) const;

            /**
             * @brief Get the last error message
             * @return Error message string
             */
            const std::string& GetLastError() const;

            /**
             * @brief Clear all quantization tables
             */
            void Clear();

        private:
            /**
             * @brief Quantization table structure
             */
            struct QuantizationTable
            {
                bool defined;               ///< Whether the table is defined
                int precision;              ///< Table precision (0 = 8-bit, 1 = 16-bit)
                uint16_t values[64];        ///< Quantization values in zigzag order
            };

            QuantizationTable m_Tables[4];  ///< Quantization tables (max 4)
            std::string m_LastError;        ///< Last error message

            // JPEG zigzag order for 8x8 blocks (same as in JPEGCoefficientDecoder)
            static const int ZIGZAG_ORDER[64];

            /**
             * @brief Set the last error message
             * @param error Error message
             */
            void SetError(const std::string& error);
        };

    } // namespace Util
} // namespace Pyramid
