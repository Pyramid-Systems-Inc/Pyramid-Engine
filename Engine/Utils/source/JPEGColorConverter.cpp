#include "Pyramid/Util/JPEGColorConverter.hpp"
#include "Pyramid/Util/Log.hpp"
#include <algorithm>

namespace Pyramid
{
    namespace Util
    {
        JPEGColorConverter::JPEGColorConverter()
            : m_HighQuality(false)
        {
        }

        void JPEGColorConverter::YCbCrToRGB(uint8_t y, uint8_t cb, uint8_t cr, uint8_t& r, uint8_t& g, uint8_t& b)
        {
            if (m_HighQuality)
            {
                YCbCrToRGBHighQuality(y, cb, cr, r, g, b);
            }
            else
            {
                YCbCrToRGBFast(y, cb, cr, r, g, b);
            }
        }

        bool JPEGColorConverter::ConvertYCbCrToRGB(const uint8_t* ycbcr, uint8_t* rgb, int pixelCount)
        {
            if (!ycbcr || !rgb)
            {
                SetError("Null pointer passed to ConvertYCbCrToRGB");
                return false;
            }

            if (pixelCount <= 0)
            {
                SetError("Invalid pixel count");
                return false;
            }

            for (int i = 0; i < pixelCount; ++i)
            {
                uint8_t y = ycbcr[i * 3];
                uint8_t cb = ycbcr[i * 3 + 1];
                uint8_t cr = ycbcr[i * 3 + 2];

                uint8_t r, g, b;
                YCbCrToRGB(y, cb, cr, r, g, b);

                rgb[i * 3] = r;
                rgb[i * 3 + 1] = g;
                rgb[i * 3 + 2] = b;
            }

            return true;
        }

        bool JPEGColorConverter::ConvertPlanarYCbCrToRGB(const uint8_t* yData, const uint8_t* cbData, 
                                                       const uint8_t* crData, uint8_t* rgbData, int pixelCount)
        {
            if (!yData || !cbData || !crData || !rgbData)
            {
                SetError("Null pointer passed to ConvertPlanarYCbCrToRGB");
                return false;
            }

            if (pixelCount <= 0)
            {
                SetError("Invalid pixel count");
                return false;
            }

            for (int i = 0; i < pixelCount; ++i)
            {
                uint8_t y = yData[i];
                uint8_t cb = cbData[i];
                uint8_t cr = crData[i];

                uint8_t r, g, b;
                YCbCrToRGB(y, cb, cr, r, g, b);

                rgbData[i * 3] = r;
                rgbData[i * 3 + 1] = g;
                rgbData[i * 3 + 2] = b;
            }

            return true;
        }

        bool JPEGColorConverter::ConvertGrayscaleToRGB(const uint8_t* yData, uint8_t* rgbData, int pixelCount)
        {
            if (!yData || !rgbData)
            {
                SetError("Null pointer passed to ConvertGrayscaleToRGB");
                return false;
            }

            if (pixelCount <= 0)
            {
                SetError("Invalid pixel count");
                return false;
            }

            for (int i = 0; i < pixelCount; ++i)
            {
                uint8_t gray = yData[i];
                rgbData[i * 3] = gray;     // R
                rgbData[i * 3 + 1] = gray; // G
                rgbData[i * 3 + 2] = gray; // B
            }

            return true;
        }

        void JPEGColorConverter::SetHighQuality(bool highQuality)
        {
            m_HighQuality = highQuality;
        }

        const std::string& JPEGColorConverter::GetLastError() const
        {
            return m_LastError;
        }

        uint8_t JPEGColorConverter::ClampToByte(int value)
        {
            if (value < 0) return 0;
            if (value > 255) return 255;
            return static_cast<uint8_t>(value);
        }

        uint8_t JPEGColorConverter::ClampToByte(float value)
        {
            if (value < 0.0f) return 0;
            if (value > 255.0f) return 255;
            return static_cast<uint8_t>(value + 0.5f); // Round to nearest
        }

        void JPEGColorConverter::YCbCrToRGBFast(uint8_t y, uint8_t cb, uint8_t cr, uint8_t& r, uint8_t& g, uint8_t& b)
        {
            // Fast integer-based conversion using fixed-point arithmetic
            // Based on ITU-R BT.601 standard
            
            // Convert to signed values (Cb and Cr are centered at 128)
            int yVal = static_cast<int>(y);
            int cbVal = static_cast<int>(cb) - 128;
            int crVal = static_cast<int>(cr) - 128;

            // Apply conversion matrix with fixed-point arithmetic (scale by 256)
            // R = Y + 1.402 * Cr
            // G = Y - 0.344 * Cb - 0.714 * Cr  
            // B = Y + 1.772 * Cb

            int rVal = yVal + ((359 * crVal) >> 8);                    // 1.402 ≈ 359/256
            int gVal = yVal - ((88 * cbVal + 183 * crVal) >> 8);       // 0.344 ≈ 88/256, 0.714 ≈ 183/256
            int bVal = yVal + ((454 * cbVal) >> 8);                    // 1.772 ≈ 454/256

            r = ClampToByte(rVal);
            g = ClampToByte(gVal);
            b = ClampToByte(bVal);
        }

        void JPEGColorConverter::YCbCrToRGBHighQuality(uint8_t y, uint8_t cb, uint8_t cr, uint8_t& r, uint8_t& g, uint8_t& b)
        {
            // High-quality floating point conversion
            // Based on ITU-R BT.601 standard with precise coefficients
            
            float yVal = static_cast<float>(y);
            float cbVal = static_cast<float>(cb) - 128.0f;
            float crVal = static_cast<float>(cr) - 128.0f;

            // Apply conversion matrix
            // R = Y + 1.402 * Cr
            // G = Y - 0.34414 * Cb - 0.71414 * Cr
            // B = Y + 1.772 * Cb

            float rVal = yVal + 1.402f * crVal;
            float gVal = yVal - 0.34414f * cbVal - 0.71414f * crVal;
            float bVal = yVal + 1.772f * cbVal;

            r = ClampToByte(rVal);
            g = ClampToByte(gVal);
            b = ClampToByte(bVal);
        }

        void JPEGColorConverter::SetError(const std::string& error)
        {
            m_LastError = error;
            PYRAMID_LOG_ERROR("JPEGColorConverter: ", error);
        }

    } // namespace Util
} // namespace Pyramid
