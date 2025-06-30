#include "Pyramid/Util/Image.hpp"
#include "Pyramid/Util/JPEGLoader.hpp"
#include <iostream>
#include <vector>

using namespace Pyramid::Util;

bool TestJPEGBasicParsing()
{
    std::cout << "=== Testing Basic JPEG Parsing ===" << std::endl;

    // Very minimal JPEG structure - just test basic marker recognition
    std::vector<uint8_t> jpegData = {
        // SOI (Start of Image)
        0xFF, 0xD8,

        // SOF0 (Start of Frame - Baseline DCT) - Minimal
        0xFF, 0xC0,
        0x00, 0x0B, // Length: 11 bytes (8 fixed + 3 for 1 component)
        0x08,       // Precision: 8 bits
        0x00, 0x20, // Height: 32 pixels
        0x00, 0x20, // Width: 32 pixels
        0x01,       // Number of components: 1 (Grayscale)
        // Component 1 (Y)
        0x01, // Component ID: 1
        0x11, // Sampling factors: 1x1
        0x00, // Quantization table ID: 0

        // EOI (End of Image) - Skip complex segments for now
        0xFF, 0xD9};

    std::cout << "DEBUG: Minimal JPEG test data size: " << jpegData.size() << " bytes" << std::endl;

    // Test loading from memory
    ImageData result = JPEGLoader::LoadFromMemory(jpegData.data(), jpegData.size());

    if (!result.Data)
    {
        std::cout << "ERROR: Failed to parse minimal JPEG markers" << std::endl;
        std::cout << "Last error: " << JPEGLoader::GetLastError() << std::endl;
        return false;
    }

    // Verify basic properties
    if (result.Width != 32 || result.Height != 32 || result.Channels != 1)
    {
        std::cout << "ERROR: Incorrect JPEG properties parsed" << std::endl;
        std::cout << "Expected: 32x32x1, Got: " << result.Width << "x" << result.Height << "x" << result.Channels << std::endl;
        Image::Free(result.Data);
        return false;
    }

    Image::Free(result.Data);
    std::cout << "SUCCESS: Basic JPEG parsing test passed!" << std::endl;
    return true;
}

bool TestJPEGExtensionRecognition()
{
    std::cout << "\n=== Testing JPEG Extension Recognition ===" << std::endl;

    // Test .jpg extension with unique filename
    ImageData result1 = Image::Load("nonexistent_unique_12345.jpg");
    if (result1.Data != nullptr)
    {
        std::cout << "ERROR: Non-existent .jpg file was loaded" << std::endl;
        Image::Free(result1.Data);
        return false;
    }

    // Test .jpeg extension with unique filename
    ImageData result2 = Image::Load("nonexistent_unique_67890.jpeg");
    if (result2.Data != nullptr)
    {
        std::cout << "ERROR: Non-existent .jpeg file was loaded" << std::endl;
        Image::Free(result2.Data);
        return false;
    }

    std::cout << "SUCCESS: JPEG extension recognition test passed!" << std::endl;
    return true;
}

int main()
{
    std::cout << "Starting Simple JPEG Tests..." << std::endl;

    bool allTestsPassed = true;

    allTestsPassed &= TestJPEGBasicParsing();
    allTestsPassed &= TestJPEGExtensionRecognition();

    std::cout << "\n=== Test Results ===" << std::endl;
    if (allTestsPassed)
    {
        std::cout << "🎉 ALL JPEG TESTS PASSED!" << std::endl;
        std::cout << "JPEG marker parsing is working correctly." << std::endl;
        std::cout << "Ready to implement Huffman decoding, IDCT, and color conversion." << std::endl;
    }
    else
    {
        std::cout << "❌ SOME TESTS FAILED! Please check the implementation." << std::endl;
    }

    return allTestsPassed ? 0 : 1;
}
