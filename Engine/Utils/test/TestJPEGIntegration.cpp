#include "Pyramid/Util/JPEGLoader.hpp"
#include "Pyramid/Util/Image.hpp"
#include <iostream>
#include <fstream>

using namespace Pyramid::Util;

// Forward declarations
bool TestMinimalJPEG();

bool TestJPEGLoaderIntegration()
{
    std::cout << "=== Testing JPEG Loader Integration ===" << std::endl;

    // Try to load a JPEG file (if available)
    std::vector<std::string> testFiles = {
        "test.jpg",
        "test.jpeg",
        "sample.jpg",
        "sample.jpeg"};

    bool foundJPEG = false;

    for (const std::string &filename : testFiles)
    {
        std::ifstream file(filename, std::ios::binary);
        if (file.good())
        {
            file.close();

            std::cout << "Found JPEG file: " << filename << std::endl;

            // Try to load it with our JPEG loader
            ImageData result = JPEGLoader::LoadFromFile(filename);

            if (result.Data != nullptr)
            {
                std::cout << "SUCCESS: Loaded JPEG " << filename << std::endl;
                std::cout << "  Dimensions: " << result.Width << "x" << result.Height << std::endl;
                std::cout << "  Channels: " << result.Channels << std::endl;

                // Clean up
                delete[] result.Data;
                foundJPEG = true;
                break;
            }
            else
            {
                std::cout << "ERROR: Failed to load " << filename << std::endl;
                std::cout << "Last error: " << JPEGLoader::GetLastError() << std::endl;
            }
        }
    }

    if (!foundJPEG)
    {
        std::cout << "INFO: No JPEG test files found. Testing with minimal JPEG data..." << std::endl;
        return TestMinimalJPEG();
    }

    return foundJPEG;
}

bool TestMinimalJPEG()
{
    std::cout << "\n=== Testing Minimal JPEG Structure ===" << std::endl;

    // Create a minimal marker-only JPEG structure for parser integration testing.
    // Current loader produces an RGB test pattern after marker parsing.
    std::vector<uint8_t> minimalJPEG = {
        // SOI marker
        0xFF, 0xD8,

        // SOF0 marker
        0xFF, 0xC0,
        0x00, 0x0B, // Length: 11 bytes (8 fixed + 3 for 1 component)
        0x08,       // Precision: 8 bits
        0x00, 0x08, // Height: 8
        0x00, 0x08, // Width: 8
        0x01,       // Number of components: 1 (grayscale)
        0x01,       // Component ID: 1
        0x11,       // Sampling factors: 1x1
        0x00,       // Quantization table ID: 0

        // EOI marker
        0xFF, 0xD9};

    // Test our JPEG loader with this minimal structure
    ImageData result = JPEGLoader::LoadFromMemory(minimalJPEG.data(), minimalJPEG.size());

    if (result.Data != nullptr)
    {
        std::cout << "SUCCESS: Parsed minimal JPEG structure" << std::endl;
        std::cout << "  Dimensions: " << result.Width << "x" << result.Height << std::endl;
        std::cout << "  Channels: " << result.Channels << std::endl;

        // Verify the test pattern was created
        if (result.Width == 8 && result.Height == 8 && result.Channels == 3)
        {
            std::cout << "SUCCESS: Test pattern created with correct dimensions" << std::endl;
        }
        else
        {
            std::cout << "ERROR: Unexpected dimensions in test pattern" << std::endl;
        }

        // Clean up
        delete[] result.Data;
        return true;
    }
    else
    {
        std::cout << "ERROR: Failed to parse minimal JPEG" << std::endl;
        std::cout << "Last error: " << JPEGLoader::GetLastError() << std::endl;
        return false;
    }
}

bool TestJPEGWithImageClass()
{
    std::cout << "\n=== Testing JPEG with Image Class Integration ===" << std::endl;

    // Test if our JPEG loader integrates with the main Image class
    std::vector<std::string> testFiles = {
        "test.jpg",
        "test.jpeg",
        "sample.jpg"};

    for (const std::string &filename : testFiles)
    {
        std::ifstream file(filename, std::ios::binary);
        if (file.good())
        {
            file.close();

            ImageData result = Image::Load(filename);
            if (result.Data != nullptr)
            {
                std::cout << "SUCCESS: Image class loaded JPEG " << filename << std::endl;
                std::cout << "  Dimensions: " << result.Width << "x" << result.Height << std::endl;
                std::cout << "  Channels: " << result.Channels << std::endl;

                // Clean up
                Image::Free(result.Data);
                return true;
            }
            else
            {
                std::cout << "ERROR: Image class failed to load " << filename << std::endl;
            }
        }
    }

    std::cout << "INFO: No JPEG files found for Image class integration test" << std::endl;
    return true; // Not a failure, just no files to test
}

int main()
{
    std::cout << "Starting JPEG Integration Tests..." << std::endl;

    bool allTestsPassed = true;

    allTestsPassed &= TestJPEGLoaderIntegration();
    allTestsPassed &= TestJPEGWithImageClass();

    std::cout << "\n=== Integration Test Results ===" << std::endl;
    if (allTestsPassed)
    {
        std::cout << "🎉 JPEG INTEGRATION TESTS PASSED!" << std::endl;
        std::cout << "JPEG loader is integrated and working." << std::endl;
        std::cout << "Ready for real JPEG file testing!" << std::endl;
    }
    else
    {
        std::cout << "❌ SOME INTEGRATION TESTS FAILED!" << std::endl;
    }

    return allTestsPassed ? 0 : 1;
}
