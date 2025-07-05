#include "Pyramid/Util/JPEGDequantizer.hpp"
#include <iostream>
#include <vector>

using namespace Pyramid::Util;

bool TestQuantizationTableSetup()
{
    std::cout << "=== Testing Quantization Table Setup ===" << std::endl;
    
    JPEGDequantizer dequantizer;
    
    // Create a simple quantization table (all values = 2 for easy testing)
    uint16_t quantTable[64];
    for (int i = 0; i < 64; ++i)
    {
        quantTable[i] = 2;
    }
    
    // Set quantization table 0
    if (!dequantizer.SetQuantizationTable(0, quantTable, 0))
    {
        std::cout << "ERROR: Failed to set quantization table" << std::endl;
        std::cout << "Last error: " << dequantizer.GetLastError() << std::endl;
        return false;
    }
    
    // Check if table is defined
    if (!dequantizer.IsTableDefined(0))
    {
        std::cout << "ERROR: Quantization table 0 should be defined" << std::endl;
        return false;
    }
    
    // Check that other tables are not defined
    if (dequantizer.IsTableDefined(1))
    {
        std::cout << "ERROR: Quantization table 1 should not be defined" << std::endl;
        return false;
    }
    
    std::cout << "SUCCESS: Quantization table setup test passed!" << std::endl;
    return true;
}

bool TestBasicDequantization()
{
    std::cout << "\n=== Testing Basic Dequantization ===" << std::endl;
    
    JPEGDequantizer dequantizer;
    
    // Create a quantization table with known values
    uint16_t quantTable[64];
    for (int i = 0; i < 64; ++i)
    {
        quantTable[i] = i + 1; // Values 1, 2, 3, ..., 64
    }
    
    if (!dequantizer.SetQuantizationTable(0, quantTable, 0))
    {
        std::cout << "ERROR: Failed to set quantization table" << std::endl;
        return false;
    }
    
    // Create test coefficients
    int16_t coefficients[64];
    for (int i = 0; i < 64; ++i)
    {
        coefficients[i] = 2; // All coefficients = 2
    }
    
    // Dequantize
    if (!dequantizer.DequantizeBlock(coefficients, 0))
    {
        std::cout << "ERROR: Failed to dequantize block" << std::endl;
        std::cout << "Last error: " << dequantizer.GetLastError() << std::endl;
        return false;
    }
    
    // Check results: coefficient[i] should be 2 * (i + 1)
    for (int i = 0; i < 64; ++i)
    {
        int16_t expected = 2 * (i + 1);
        if (coefficients[i] != expected)
        {
            std::cout << "ERROR: Coefficient " << i << " should be " << expected 
                      << ", got " << coefficients[i] << std::endl;
            return false;
        }
    }
    
    std::cout << "SUCCESS: Basic dequantization test passed!" << std::endl;
    return true;
}

bool TestDequantizationWithNegativeCoefficients()
{
    std::cout << "\n=== Testing Dequantization with Negative Coefficients ===" << std::endl;
    
    JPEGDequantizer dequantizer;
    
    // Simple quantization table
    uint16_t quantTable[64];
    for (int i = 0; i < 64; ++i)
    {
        quantTable[i] = 3; // All values = 3
    }
    
    if (!dequantizer.SetQuantizationTable(1, quantTable, 0))
    {
        std::cout << "ERROR: Failed to set quantization table" << std::endl;
        return false;
    }
    
    // Test coefficients with positive and negative values
    int16_t coefficients[64];
    for (int i = 0; i < 64; ++i)
    {
        coefficients[i] = (i % 2 == 0) ? 4 : -4; // Alternating +4, -4
    }
    
    // Dequantize
    if (!dequantizer.DequantizeBlock(coefficients, 1))
    {
        std::cout << "ERROR: Failed to dequantize block with negative coefficients" << std::endl;
        return false;
    }
    
    // Check results: should be +12 or -12
    for (int i = 0; i < 64; ++i)
    {
        int16_t expected = (i % 2 == 0) ? 12 : -12;
        if (coefficients[i] != expected)
        {
            std::cout << "ERROR: Coefficient " << i << " should be " << expected 
                      << ", got " << coefficients[i] << std::endl;
            return false;
        }
    }
    
    std::cout << "SUCCESS: Negative coefficients dequantization test passed!" << std::endl;
    return true;
}

bool TestErrorHandling()
{
    std::cout << "\n=== Testing Error Handling ===" << std::endl;
    
    JPEGDequantizer dequantizer;
    
    // Test invalid table ID
    uint16_t quantTable[64];
    for (int i = 0; i < 64; ++i)
    {
        quantTable[i] = 1;
    }
    
    if (dequantizer.SetQuantizationTable(5, quantTable, 0))
    {
        std::cout << "ERROR: Should have failed with invalid table ID" << std::endl;
        return false;
    }
    
    // Test dequantization with undefined table
    int16_t coefficients[64] = {0};
    if (dequantizer.DequantizeBlock(coefficients, 0))
    {
        std::cout << "ERROR: Should have failed with undefined table" << std::endl;
        return false;
    }
    
    // Test with zero quantization value
    quantTable[0] = 0; // Invalid zero value
    if (dequantizer.SetQuantizationTable(0, quantTable, 0))
    {
        std::cout << "ERROR: Should have failed with zero quantization value" << std::endl;
        return false;
    }
    
    std::cout << "SUCCESS: Error handling test passed!" << std::endl;
    return true;
}

bool TestOverflowProtection()
{
    std::cout << "\n=== Testing Overflow Protection ===" << std::endl;
    
    JPEGDequantizer dequantizer;
    
    // Create quantization table with large values
    uint16_t quantTable[64];
    for (int i = 0; i < 64; ++i)
    {
        quantTable[i] = 1000; // Large quantization values
    }
    
    if (!dequantizer.SetQuantizationTable(0, quantTable, 0))
    {
        std::cout << "ERROR: Failed to set quantization table" << std::endl;
        return false;
    }
    
    // Test coefficients that would cause overflow
    int16_t coefficients[64];
    for (int i = 0; i < 64; ++i)
    {
        coefficients[i] = 100; // 100 * 1000 = 100,000 (exceeds int16_t range)
    }
    
    // Dequantize (should clamp to int16_t range)
    if (!dequantizer.DequantizeBlock(coefficients, 0))
    {
        std::cout << "ERROR: Failed to dequantize block" << std::endl;
        return false;
    }
    
    // Check that values are clamped to INT16_MAX
    for (int i = 0; i < 64; ++i)
    {
        if (coefficients[i] != INT16_MAX)
        {
            std::cout << "ERROR: Coefficient " << i << " should be clamped to " << INT16_MAX 
                      << ", got " << coefficients[i] << std::endl;
            return false;
        }
    }
    
    std::cout << "SUCCESS: Overflow protection test passed!" << std::endl;
    return true;
}

int main()
{
    std::cout << "Starting JPEG Dequantizer Tests..." << std::endl;
    
    bool allTestsPassed = true;
    
    allTestsPassed &= TestQuantizationTableSetup();
    allTestsPassed &= TestBasicDequantization();
    allTestsPassed &= TestDequantizationWithNegativeCoefficients();
    allTestsPassed &= TestErrorHandling();
    allTestsPassed &= TestOverflowProtection();
    
    std::cout << "\n=== Test Results ===" << std::endl;
    if (allTestsPassed)
    {
        std::cout << "🎉 ALL DEQUANTIZER TESTS PASSED!" << std::endl;
        std::cout << "JPEG dequantization is working correctly." << std::endl;
        std::cout << "Ready to implement IDCT (Inverse DCT)." << std::endl;
    }
    else
    {
        std::cout << "❌ SOME TESTS FAILED! Please check the implementation." << std::endl;
    }
    
    return allTestsPassed ? 0 : 1;
}
