#pragma once

#include <cstdint>
#include <string>

namespace Pyramid
{
    namespace Util
    {
        /**
         * @brief JPEG Inverse Discrete Cosine Transform (IDCT) implementation
         * 
         * This class handles the IDCT operation for JPEG decoding, converting
         * frequency domain coefficients back to spatial domain pixel values.
         * Uses a high-quality implementation with proper mathematical precision.
         */
        class JPEGIDCT
        {
        public:
            /**
             * @brief Construct a JPEG IDCT processor
             */
            JPEGIDCT();

            /**
             * @brief Perform IDCT on an 8x8 block of coefficients
             * 
             * @param coefficients Input coefficients in zigzag order (64 values)
             * @param output Output pixel values (64 values, range 0-255)
             * @return true if successful, false otherwise
             */
            bool ProcessBlock(const int16_t coefficients[64], uint8_t output[64]);

            /**
             * @brief Perform IDCT on an 8x8 block with floating point output
             * 
             * @param coefficients Input coefficients in zigzag order (64 values)
             * @param output Output pixel values (64 values, floating point)
             * @return true if successful, false otherwise
             */
            bool ProcessBlockFloat(const int16_t coefficients[64], float output[64]);

            /**
             * @brief Get the last error message
             * @return Error message string
             */
            const std::string& GetLastError() const;

        private:
            std::string m_LastError; ///< Last error message

            // JPEG zigzag order for 8x8 blocks
            static const int ZIGZAG_ORDER[64];

            // IDCT constants and tables
            static const float IDCT_CONSTANTS[64];
            static const float COS_TABLE[8][8];

            /**
             * @brief Convert zigzag ordered coefficients to 8x8 matrix
             * @param zigzag Input coefficients in zigzag order
             * @param matrix Output 8x8 matrix
             */
            void ZigzagToMatrix(const int16_t zigzag[64], float matrix[8][8]);

            /**
             * @brief Perform 1D IDCT on a row or column
             * @param input Input 8 values
             * @param output Output 8 values
             */
            void IDCT1D(const float input[8], float output[8]);

            /**
             * @brief Perform 2D IDCT on an 8x8 matrix
             * @param input Input 8x8 matrix
             * @param output Output 8x8 matrix
             */
            void IDCT2D(const float input[8][8], float output[8][8]);

            /**
             * @brief Clamp and convert float to uint8_t with level shift
             * @param value Input floating point value
             * @return Clamped uint8_t value (0-255)
             */
            uint8_t ClampAndConvert(float value);

            /**
             * @brief Set the last error message
             * @param error Error message
             */
            void SetError(const std::string& error);
        };

    } // namespace Util
} // namespace Pyramid
