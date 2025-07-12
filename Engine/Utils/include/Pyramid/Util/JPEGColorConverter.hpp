#pragma once

#include <cstdint>
#include <string>

namespace Pyramid
{
    namespace Util
    {
        /**
         * @brief JPEG color space conversion utilities
         * 
         * This class handles color space conversions for JPEG decoding,
         * primarily YCbCr to RGB conversion with proper clamping and
         * mathematical precision.
         */
        class JPEGColorConverter
        {
        public:
            /**
             * @brief Construct a JPEG color converter
             */
            JPEGColorConverter();

            /**
             * @brief Convert YCbCr to RGB for a single pixel
             * 
             * @param y Luminance component (0-255)
             * @param cb Blue chrominance component (0-255)
             * @param cr Red chrominance component (0-255)
             * @param r Output red component (0-255)
             * @param g Output green component (0-255)
             * @param b Output blue component (0-255)
             */
            void YCbCrToRGB(uint8_t y, uint8_t cb, uint8_t cr, uint8_t& r, uint8_t& g, uint8_t& b);

            /**
             * @brief Convert YCbCr to RGB for an array of pixels
             * 
             * @param ycbcr Input YCbCr data (interleaved: Y, Cb, Cr, Y, Cb, Cr, ...)
             * @param rgb Output RGB data (interleaved: R, G, B, R, G, B, ...)
             * @param pixelCount Number of pixels to convert
             * @return true if successful, false otherwise
             */
            bool ConvertYCbCrToRGB(const uint8_t* ycbcr, uint8_t* rgb, int pixelCount);

            /**
             * @brief Convert YCbCr to RGB for separate component arrays
             * 
             * @param yData Y component array
             * @param cbData Cb component array  
             * @param crData Cr component array
             * @param rgbData Output RGB data (interleaved)
             * @param pixelCount Number of pixels to convert
             * @return true if successful, false otherwise
             */
            bool ConvertPlanarYCbCrToRGB(const uint8_t* yData, const uint8_t* cbData, 
                                       const uint8_t* crData, uint8_t* rgbData, int pixelCount);

            /**
             * @brief Convert grayscale (Y-only) to RGB
             * 
             * @param yData Y component array
             * @param rgbData Output RGB data (interleaved)
             * @param pixelCount Number of pixels to convert
             * @return true if successful, false otherwise
             */
            bool ConvertGrayscaleToRGB(const uint8_t* yData, uint8_t* rgbData, int pixelCount);

            /**
             * @brief Set conversion quality mode
             * 
             * @param highQuality If true, uses floating point math for better precision
             */
            void SetHighQuality(bool highQuality);

            /**
             * @brief Get the last error message
             * @return Error message string
             */
            const std::string& GetLastError() const;

        private:
            bool m_HighQuality;      ///< Use high-quality floating point conversion
            std::string m_LastError; ///< Last error message

            /**
             * @brief Clamp value to uint8_t range [0, 255]
             * @param value Input value
             * @return Clamped uint8_t value
             */
            uint8_t ClampToByte(int value);

            /**
             * @brief Clamp floating point value to uint8_t range [0, 255]
             * @param value Input floating point value
             * @return Clamped uint8_t value
             */
            uint8_t ClampToByte(float value);

            /**
             * @brief Fast integer-based YCbCr to RGB conversion
             * @param y Y component (0-255)
             * @param cb Cb component (0-255)
             * @param cr Cr component (0-255)
             * @param r Output red component
             * @param g Output green component
             * @param b Output blue component
             */
            void YCbCrToRGBFast(uint8_t y, uint8_t cb, uint8_t cr, uint8_t& r, uint8_t& g, uint8_t& b);

            /**
             * @brief High-quality floating point YCbCr to RGB conversion
             * @param y Y component (0-255)
             * @param cb Cb component (0-255)
             * @param cr Cr component (0-255)
             * @param r Output red component
             * @param g Output green component
             * @param b Output blue component
             */
            void YCbCrToRGBHighQuality(uint8_t y, uint8_t cb, uint8_t cr, uint8_t& r, uint8_t& g, uint8_t& b);

            /**
             * @brief Set the last error message
             * @param error Error message
             */
            void SetError(const std::string& error);
        };

    } // namespace Util
} // namespace Pyramid
