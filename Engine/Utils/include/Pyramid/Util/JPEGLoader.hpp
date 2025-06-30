#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <array>

namespace Pyramid
{
    namespace Util
    {
        struct ImageData; // Forward declaration

        /**
         * @brief JPEG image loader implementation
         * 
         * This class implements JPEG image loading according to the JPEG/JFIF specification.
         * It supports:
         * - Baseline DCT (SOF0) images
         * - Huffman-encoded data
         * - YCbCr and grayscale color spaces
         * - 8-bit precision
         * - Progressive JPEG (future enhancement)
         */
        class JPEGLoader
        {
        public:
            /**
             * @brief Load a JPEG image from file
             * 
             * @param filepath Path to the JPEG file
             * @return ImageData structure with loaded image, or empty structure on failure
             */
            static ImageData LoadFromFile(const std::string& filepath);

            /**
             * @brief Load a JPEG image from memory buffer
             * 
             * @param data Pointer to JPEG data in memory
             * @param size Size of the JPEG data in bytes
             * @return ImageData structure with loaded image, or empty structure on failure
             */
            static ImageData LoadFromMemory(const uint8_t* data, size_t size);

            /**
             * @brief Get the last error message
             * @return Error message string
             */
            static const std::string& GetLastError();

        private:
            static std::string s_LastError;

            // JPEG marker definitions
            enum JPEGMarker : uint16_t
            {
                SOI  = 0xFFD8,  // Start of Image
                EOI  = 0xFFD9,  // End of Image
                SOF0 = 0xFFC0,  // Start of Frame (Baseline DCT)
                SOF1 = 0xFFC1,  // Start of Frame (Extended Sequential DCT)
                SOF2 = 0xFFC2,  // Start of Frame (Progressive DCT)
                DHT  = 0xFFC4,  // Define Huffman Table
                DQT  = 0xFFDB,  // Define Quantization Table
                DRI  = 0xFFDD,  // Define Restart Interval
                SOS  = 0xFFDA,  // Start of Scan
                APP0 = 0xFFE0,  // Application Segment 0 (JFIF)
                APP1 = 0xFFE1,  // Application Segment 1 (EXIF)
                COM  = 0xFFFE   // Comment
            };

            // JPEG frame header (SOF0)
            struct FrameHeader
            {
                uint8_t precision;          // Sample precision (usually 8)
                uint16_t height;            // Image height
                uint16_t width;             // Image width
                uint8_t numComponents;      // Number of components (1=grayscale, 3=YCbCr)
                
                struct Component
                {
                    uint8_t id;             // Component ID (1=Y, 2=Cb, 3=Cr)
                    uint8_t samplingFactor; // Horizontal and vertical sampling factors
                    uint8_t quantTableId;   // Quantization table ID
                } components[3];            // Max 3 components for YCbCr
            };

            // JPEG scan header (SOS)
            struct ScanHeader
            {
                uint8_t numComponents;      // Number of components in scan
                
                struct Component
                {
                    uint8_t id;             // Component ID
                    uint8_t huffmanTableId; // DC and AC Huffman table IDs
                } components[3];
                
                uint8_t spectralStart;      // Start of spectral selection (0 for baseline)
                uint8_t spectralEnd;        // End of spectral selection (63 for baseline)
                uint8_t approximation;      // Successive approximation (0 for baseline)
            };

            // Quantization table
            struct QuantizationTable
            {
                bool defined;
                uint8_t precision;          // 0 = 8-bit, 1 = 16-bit
                uint16_t values[64];        // Quantization values in zigzag order
            };

            // Huffman table
            struct HuffmanTable
            {
                bool defined;
                uint8_t codeLengths[16];    // Number of codes of each length
                uint8_t symbols[256];       // Symbols in order
                uint8_t numSymbols;         // Total number of symbols
            };

            // JPEG decoder state
            struct JPEGDecoder
            {
                FrameHeader frameHeader;
                ScanHeader scanHeader;
                QuantizationTable quantTables[4];   // Max 4 quantization tables
                HuffmanTable dcTables[4];           // DC Huffman tables
                HuffmanTable acTables[4];           // AC Huffman tables
                uint16_t restartInterval;           // Restart interval (0 = no restart)
            };

            /**
             * @brief Internal JPEG loading implementation
             * 
             * @param data Pointer to JPEG data
             * @param size Size of JPEG data
             * @return ImageData structure
             */
            static ImageData LoadJPEGInternal(const uint8_t* data, size_t size);

            /**
             * @brief Parse JPEG markers and segments
             * 
             * @param data Pointer to JPEG data
             * @param size Size of data
             * @param decoder Output decoder state
             * @param imageDataStart Output pointer to start of image data
             * @param imageDataSize Output size of image data
             * @return true if parsing was successful
             */
            static bool ParseJPEGMarkers(const uint8_t* data, size_t size, 
                                       JPEGDecoder& decoder,
                                       const uint8_t*& imageDataStart,
                                       size_t& imageDataSize);

            /**
             * @brief Parse SOF0 (Start of Frame) segment
             * 
             * @param data Segment data
             * @param size Segment size
             * @param frameHeader Output frame header
             * @return true if successful
             */
            static bool ParseSOF0(const uint8_t* data, size_t size, FrameHeader& frameHeader);

            /**
             * @brief Parse DQT (Define Quantization Table) segment
             * 
             * @param data Segment data
             * @param size Segment size
             * @param quantTables Output quantization tables array
             * @return true if successful
             */
            static bool ParseDQT(const uint8_t* data, size_t size, QuantizationTable quantTables[4]);

            /**
             * @brief Parse DHT (Define Huffman Table) segment
             * 
             * @param data Segment data
             * @param size Segment size
             * @param dcTables Output DC Huffman tables
             * @param acTables Output AC Huffman tables
             * @return true if successful
             */
            static bool ParseDHT(const uint8_t* data, size_t size, 
                                HuffmanTable dcTables[4], HuffmanTable acTables[4]);

            /**
             * @brief Parse SOS (Start of Scan) segment
             * 
             * @param data Segment data
             * @param size Segment size
             * @param scanHeader Output scan header
             * @return true if successful
             */
            static bool ParseSOS(const uint8_t* data, size_t size, ScanHeader& scanHeader);

            /**
             * @brief Read a 16-bit big-endian value
             * 
             * @param data Pointer to data
             * @return 16-bit value
             */
            static uint16_t ReadBigEndian16(const uint8_t* data);

            /**
             * @brief Set the last error message
             * @param error Error message
             */
            static void SetError(const std::string& error);
        };

    } // namespace Util
} // namespace Pyramid
