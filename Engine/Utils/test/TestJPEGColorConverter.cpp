#include "Pyramid/Util/JPEGColorConverter.hpp"
#include <iostream>
#include <vector>
#include <cmath>

using namespace Pyramid::Util;

bool TestBasicYCbCrToRGB()
{
    std::cout << "=== Testing Basic YCbCr to RGB Conversion ===" << std::endl;
    
    JPEGColorConverter converter;
    
    // Test pure white (Y=255, Cb=128, Cr=128)
    uint8_t r, g, b;
    converter.YCbCrToRGB(255, 128, 128, r, g, b);
    
    if (r != 255 || g != 255 || b != 255)
    {
        std::cout << "ERROR: Pure white conversion failed" << std::endl;
        std::cout << "Expected (255,255,255), got (" << static_cast<int>(r) << "," 
                  << static_cast<int>(g) << "," << static_cast<int>(b) << ")" << std::endl;
        return false;
    }
    
    // Test pure black (Y=0, Cb=128, Cr=128)
    converter.YCbCrToRGB(0, 128, 128, r, g, b);
    
    if (r != 0 || g != 0 || b != 0)
    {
        std::cout << "ERROR: Pure black conversion failed" << std::endl;
        std::cout << "Expected (0,0,0), got (" << static_cast<int>(r) << "," 
                  << static_cast<int>(g) << "," << static_cast<int>(b) << ")" << std::endl;
        return false;
    }
    
    // Test middle gray (Y=128, Cb=128, Cr=128)
    converter.YCbCrToRGB(128, 128, 128, r, g, b);
    
    if (abs(static_cast<int>(r) - 128) > 2 || 
        abs(static_cast<int>(g) - 128) > 2 || 
        abs(static_cast<int>(b) - 128) > 2)
    {
        std::cout << "ERROR: Middle gray conversion failed" << std::endl;
        std::cout << "Expected ~(128,128,128), got (" << static_cast<int>(r) << "," 
                  << static_cast<int>(g) << "," << static_cast<int>(b) << ")" << std::endl;
        return false;
    }
    
    std::cout << "SUCCESS: Basic YCbCr to RGB conversions work correctly" << std::endl;
    return true;
}

bool TestColorfulConversions()
{
    std::cout << "\n=== Testing Colorful YCbCr Conversions ===" << std::endl;
    
    JPEGColorConverter converter;
    uint8_t r, g, b;
    
    // Test red-ish color (high Cr)
    converter.YCbCrToRGB(128, 128, 200, r, g, b);
    
    if (r <= g || r <= b)
    {
        std::cout << "ERROR: High Cr should produce red-dominant color" << std::endl;
        std::cout << "Got RGB(" << static_cast<int>(r) << "," 
                  << static_cast<int>(g) << "," << static_cast<int>(b) << ")" << std::endl;
        return false;
    }
    
    // Test blue-ish color (high Cb)
    converter.YCbCrToRGB(128, 200, 128, r, g, b);
    
    if (b <= r || b <= g)
    {
        std::cout << "ERROR: High Cb should produce blue-dominant color" << std::endl;
        std::cout << "Got RGB(" << static_cast<int>(r) << "," 
                  << static_cast<int>(g) << "," << static_cast<int>(b) << ")" << std::endl;
        return false;
    }
    
    std::cout << "SUCCESS: Colorful conversions show expected color dominance" << std::endl;
    return true;
}

bool TestArrayConversion()
{
    std::cout << "\n=== Testing Array Conversion ===" << std::endl;
    
    JPEGColorConverter converter;
    
    // Test interleaved YCbCr to RGB conversion
    uint8_t ycbcr[] = {
        255, 128, 128,  // White
        0, 128, 128,    // Black  
        128, 128, 128,  // Gray
        128, 128, 200   // Reddish
    };
    
    uint8_t rgb[12]; // 4 pixels * 3 components
    
    if (!converter.ConvertYCbCrToRGB(ycbcr, rgb, 4))
    {
        std::cout << "ERROR: Array conversion failed" << std::endl;
        std::cout << "Last error: " << converter.GetLastError() << std::endl;
        return false;
    }
    
    // Check white pixel
    if (rgb[0] != 255 || rgb[1] != 255 || rgb[2] != 255)
    {
        std::cout << "ERROR: First pixel (white) conversion failed in array" << std::endl;
        return false;
    }
    
    // Check black pixel
    if (rgb[3] != 0 || rgb[4] != 0 || rgb[5] != 0)
    {
        std::cout << "ERROR: Second pixel (black) conversion failed in array" << std::endl;
        return false;
    }
    
    std::cout << "SUCCESS: Array conversion works correctly" << std::endl;
    return true;
}

bool TestPlanarConversion()
{
    std::cout << "\n=== Testing Planar Conversion ===" << std::endl;
    
    JPEGColorConverter converter;
    
    // Test separate Y, Cb, Cr arrays
    uint8_t yData[] = {255, 0, 128, 128};
    uint8_t cbData[] = {128, 128, 128, 200};
    uint8_t crData[] = {128, 128, 128, 128};
    
    uint8_t rgb[12]; // 4 pixels * 3 components
    
    if (!converter.ConvertPlanarYCbCrToRGB(yData, cbData, crData, rgb, 4))
    {
        std::cout << "ERROR: Planar conversion failed" << std::endl;
        std::cout << "Last error: " << converter.GetLastError() << std::endl;
        return false;
    }
    
    // Check white pixel (first pixel)
    if (rgb[0] != 255 || rgb[1] != 255 || rgb[2] != 255)
    {
        std::cout << "ERROR: First pixel conversion failed in planar mode" << std::endl;
        return false;
    }
    
    // Check that blue-ish pixel (high Cb) has blue dominance
    if (rgb[11] <= rgb[9] || rgb[11] <= rgb[10]) // B <= R or B <= G
    {
        std::cout << "ERROR: High Cb pixel should be blue-dominant" << std::endl;
        return false;
    }
    
    std::cout << "SUCCESS: Planar conversion works correctly" << std::endl;
    return true;
}

bool TestGrayscaleConversion()
{
    std::cout << "\n=== Testing Grayscale Conversion ===" << std::endl;
    
    JPEGColorConverter converter;
    
    uint8_t yData[] = {0, 64, 128, 192, 255};
    uint8_t rgb[15]; // 5 pixels * 3 components
    
    if (!converter.ConvertGrayscaleToRGB(yData, rgb, 5))
    {
        std::cout << "ERROR: Grayscale conversion failed" << std::endl;
        std::cout << "Last error: " << converter.GetLastError() << std::endl;
        return false;
    }
    
    // Check that R, G, B are all equal for each pixel
    for (int i = 0; i < 5; ++i)
    {
        uint8_t expectedValue = yData[i];
        if (rgb[i * 3] != expectedValue || 
            rgb[i * 3 + 1] != expectedValue || 
            rgb[i * 3 + 2] != expectedValue)
        {
            std::cout << "ERROR: Grayscale pixel " << i << " not converted correctly" << std::endl;
            std::cout << "Expected (" << static_cast<int>(expectedValue) << "," 
                      << static_cast<int>(expectedValue) << "," << static_cast<int>(expectedValue) << ")" << std::endl;
            std::cout << "Got (" << static_cast<int>(rgb[i * 3]) << "," 
                      << static_cast<int>(rgb[i * 3 + 1]) << "," << static_cast<int>(rgb[i * 3 + 2]) << ")" << std::endl;
            return false;
        }
    }
    
    std::cout << "SUCCESS: Grayscale conversion works correctly" << std::endl;
    return true;
}

bool TestQualityModes()
{
    std::cout << "\n=== Testing Quality Modes ===" << std::endl;
    
    JPEGColorConverter converter;
    
    // Test the same conversion in both quality modes
    uint8_t y = 128, cb = 150, cr = 180;
    uint8_t r1, g1, b1, r2, g2, b2;
    
    // Fast mode
    converter.SetHighQuality(false);
    converter.YCbCrToRGB(y, cb, cr, r1, g1, b1);
    
    // High quality mode
    converter.SetHighQuality(true);
    converter.YCbCrToRGB(y, cb, cr, r2, g2, b2);
    
    // Results should be similar but may differ slightly
    int diffR = abs(static_cast<int>(r1) - static_cast<int>(r2));
    int diffG = abs(static_cast<int>(g1) - static_cast<int>(g2));
    int diffB = abs(static_cast<int>(b1) - static_cast<int>(b2));
    
    if (diffR > 5 || diffG > 5 || diffB > 5)
    {
        std::cout << "WARN: Large difference between quality modes" << std::endl;
        std::cout << "Fast: (" << static_cast<int>(r1) << "," << static_cast<int>(g1) << "," << static_cast<int>(b1) << ")" << std::endl;
        std::cout << "HQ:   (" << static_cast<int>(r2) << "," << static_cast<int>(g2) << "," << static_cast<int>(b2) << ")" << std::endl;
    }
    else
    {
        std::cout << "SUCCESS: Both quality modes produce similar results" << std::endl;
    }
    
    return true;
}

bool TestErrorHandling()
{
    std::cout << "\n=== Testing Error Handling ===" << std::endl;
    
    JPEGColorConverter converter;
    
    uint8_t ycbcr[3] = {128, 128, 128};
    uint8_t rgb[3];
    
    // Test null pointers
    if (converter.ConvertYCbCrToRGB(nullptr, rgb, 1))
    {
        std::cout << "ERROR: Should have failed with null YCbCr pointer" << std::endl;
        return false;
    }
    
    if (converter.ConvertYCbCrToRGB(ycbcr, nullptr, 1))
    {
        std::cout << "ERROR: Should have failed with null RGB pointer" << std::endl;
        return false;
    }
    
    // Test invalid pixel count
    if (converter.ConvertYCbCrToRGB(ycbcr, rgb, 0))
    {
        std::cout << "ERROR: Should have failed with zero pixel count" << std::endl;
        return false;
    }
    
    if (converter.ConvertYCbCrToRGB(ycbcr, rgb, -1))
    {
        std::cout << "ERROR: Should have failed with negative pixel count" << std::endl;
        return false;
    }
    
    std::cout << "SUCCESS: Error handling works correctly" << std::endl;
    return true;
}

int main()
{
    std::cout << "Starting JPEG Color Converter Tests..." << std::endl;
    
    bool allTestsPassed = true;
    
    allTestsPassed &= TestBasicYCbCrToRGB();
    allTestsPassed &= TestColorfulConversions();
    allTestsPassed &= TestArrayConversion();
    allTestsPassed &= TestPlanarConversion();
    allTestsPassed &= TestGrayscaleConversion();
    allTestsPassed &= TestQualityModes();
    allTestsPassed &= TestErrorHandling();
    
    std::cout << "\n=== Test Results ===" << std::endl;
    if (allTestsPassed)
    {
        std::cout << "🎉 ALL COLOR CONVERTER TESTS PASSED!" << std::endl;
        std::cout << "JPEG YCbCr to RGB conversion is working correctly." << std::endl;
        std::cout << "Ready to integrate complete JPEG loader." << std::endl;
    }
    else
    {
        std::cout << "❌ SOME TESTS FAILED! Please check the implementation." << std::endl;
    }
    
    return allTestsPassed ? 0 : 1;
}
