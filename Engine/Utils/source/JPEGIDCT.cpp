#include "Pyramid/Util/JPEGIDCT.hpp"
#include "Pyramid/Util/Log.hpp"
#include <cmath>
#include <algorithm>

namespace Pyramid
{
    namespace Util
    {
        // JPEG zigzag order for 8x8 DCT blocks
        const int JPEGIDCT::ZIGZAG_ORDER[64] = {
             0,  1,  8, 16,  9,  2,  3, 10,
            17, 24, 32, 25, 18, 11,  4,  5,
            12, 19, 26, 33, 40, 48, 41, 34,
            27, 20, 13,  6,  7, 14, 21, 28,
            35, 42, 49, 56, 57, 50, 43, 36,
            29, 22, 15, 23, 30, 37, 44, 51,
            58, 59, 52, 45, 38, 31, 39, 46,
            53, 60, 61, 54, 47, 55, 62, 63
        };

        // Precomputed cosine table for IDCT
        const float JPEGIDCT::COS_TABLE[8][8] = {
            {0.3536f, 0.3536f, 0.3536f, 0.3536f, 0.3536f, 0.3536f, 0.3536f, 0.3536f},
            {0.4904f, 0.4157f, 0.2778f, 0.0975f, -0.0975f, -0.2778f, -0.4157f, -0.4904f},
            {0.4619f, 0.1913f, -0.1913f, -0.4619f, -0.4619f, -0.1913f, 0.1913f, 0.4619f},
            {0.4157f, -0.0975f, -0.4904f, -0.2778f, 0.2778f, 0.4904f, 0.0975f, -0.4157f},
            {0.3536f, -0.3536f, -0.3536f, 0.3536f, 0.3536f, -0.3536f, -0.3536f, 0.3536f},
            {0.2778f, -0.4904f, 0.0975f, 0.4157f, -0.4157f, -0.0975f, 0.4904f, -0.2778f},
            {0.1913f, -0.4619f, 0.4619f, -0.1913f, -0.1913f, 0.4619f, -0.4619f, 0.1913f},
            {0.0975f, -0.2778f, 0.4157f, -0.4904f, 0.4904f, -0.4157f, 0.2778f, -0.0975f}
        };

        JPEGIDCT::JPEGIDCT()
        {
        }

        bool JPEGIDCT::ProcessBlock(const int16_t coefficients[64], uint8_t output[64])
        {
            if (!coefficients || !output)
            {
                SetError("Null pointer passed to ProcessBlock");
                return false;
            }

            // Convert to floating point and perform IDCT
            float floatOutput[64];
            if (!ProcessBlockFloat(coefficients, floatOutput))
            {
                return false;
            }

            // Convert to uint8_t with clamping and level shift
            for (int i = 0; i < 64; ++i)
            {
                output[i] = ClampAndConvert(floatOutput[i]);
            }

            return true;
        }

        bool JPEGIDCT::ProcessBlockFloat(const int16_t coefficients[64], float output[64])
        {
            if (!coefficients || !output)
            {
                SetError("Null pointer passed to ProcessBlockFloat");
                return false;
            }

            // Convert zigzag coefficients to 8x8 matrix
            float matrix[8][8];
            ZigzagToMatrix(coefficients, matrix);

            // Perform 2D IDCT
            float idctResult[8][8];
            IDCT2D(matrix, idctResult);

            // Convert back to linear array
            for (int y = 0; y < 8; ++y)
            {
                for (int x = 0; x < 8; ++x)
                {
                    output[y * 8 + x] = idctResult[y][x];
                }
            }

            return true;
        }

        const std::string& JPEGIDCT::GetLastError() const
        {
            return m_LastError;
        }

        void JPEGIDCT::ZigzagToMatrix(const int16_t zigzag[64], float matrix[8][8])
        {
            for (int i = 0; i < 64; ++i)
            {
                int row = ZIGZAG_ORDER[i] / 8;
                int col = ZIGZAG_ORDER[i] % 8;
                matrix[row][col] = static_cast<float>(zigzag[i]);
            }
        }

        void JPEGIDCT::IDCT1D(const float input[8], float output[8])
        {
            // 1D IDCT using precomputed cosine table
            for (int x = 0; x < 8; ++x)
            {
                float sum = 0.0f;
                for (int u = 0; u < 8; ++u)
                {
                    sum += input[u] * COS_TABLE[u][x];
                }
                output[x] = sum;
            }
        }

        void JPEGIDCT::IDCT2D(const float input[8][8], float output[8][8])
        {
            // Temporary array for intermediate results
            float temp[8][8];

            // First pass: IDCT on rows
            for (int y = 0; y < 8; ++y)
            {
                IDCT1D(input[y], temp[y]);
            }

            // Second pass: IDCT on columns
            for (int x = 0; x < 8; ++x)
            {
                float column[8];
                float result[8];

                // Extract column
                for (int y = 0; y < 8; ++y)
                {
                    column[y] = temp[y][x];
                }

                // Apply IDCT to column
                IDCT1D(column, result);

                // Store result back
                for (int y = 0; y < 8; ++y)
                {
                    output[y][x] = result[y];
                }
            }
        }

        uint8_t JPEGIDCT::ClampAndConvert(float value)
        {
            // Add 128 for level shift (JPEG uses signed values, output needs unsigned)
            value += 128.0f;

            // Clamp to valid range
            if (value < 0.0f)
                return 0;
            if (value > 255.0f)
                return 255;

            return static_cast<uint8_t>(value + 0.5f); // Round to nearest integer
        }

        void JPEGIDCT::SetError(const std::string& error)
        {
            m_LastError = error;
            PYRAMID_LOG_ERROR("JPEGIDCT: ", error);
        }

    } // namespace Util
} // namespace Pyramid
