#include "Pyramid/Util/JPEGHuffmanDecoder.hpp"
#include "Pyramid/Util/BitReader.hpp"
#include <iostream>
#include <vector>

using namespace Pyramid::Util;

bool TestJPEGHuffmanTreeConstruction()
{
    std::cout << "=== Testing JPEG Huffman Tree Construction ===" << std::endl;

    // Create a simple Huffman table for testing
    // This represents a minimal DC table with 2 symbols
    uint8_t codeLengths[16] = {
        2, 0, 0, 0, 0, 0, 0, 0, // 2 codes of length 1
        0, 0, 0, 0, 0, 0, 0, 0  // No codes of other lengths
    };

    uint8_t symbols[2] = {0x00, 0x01}; // Two symbols: 0 and 1

    JPEGHuffmanDecoder decoder;

    if (!decoder.BuildTree(codeLengths, symbols, 2))
    {
        std::cout << "ERROR: Failed to build JPEG Huffman tree" << std::endl;
        return false;
    }

    if (!decoder.IsValid())
    {
        std::cout << "ERROR: JPEG Huffman tree is not valid" << std::endl;
        return false;
    }

    std::cout << "SUCCESS: JPEG Huffman tree construction test passed!" << std::endl;
    return true;
}

bool TestJPEGHuffmanDecoding()
{
    std::cout << "\n=== Testing JPEG Huffman Decoding ===" << std::endl;

    // Create a simple Huffman table
    // Code lengths: 2 codes of length 1
    uint8_t codeLengths[16] = {
        2, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};

    uint8_t symbols[2] = {0x00, 0x01}; // Symbols 0 and 1

    JPEGHuffmanDecoder decoder;
    if (!decoder.BuildTree(codeLengths, symbols, 2))
    {
        std::cout << "ERROR: Failed to build Huffman tree for decoding test" << std::endl;
        return false;
    }

    // For a canonical Huffman tree with 2 codes of length 1:
    // Code 0 gets code 0 (binary: 0)
    // Code 1 gets code 1 (binary: 1)
    // Test data: For 2 codes of length 1, we need 2 separate tests
    // Test code 0 (bit 0)
    uint8_t testData1[] = {0x00}; // 0b00000000 - LSB is 0
    BitReader bitReader1(testData1, 1);

    int symbol1 = decoder.DecodeSymbol(bitReader1);
    if (symbol1 != 0)
    {
        std::cout << "ERROR: Expected symbol 0, got " << symbol1 << std::endl;
        return false;
    }

    // Test code 1 (bit 1)
    uint8_t testData2[] = {0x01}; // 0b00000001 - LSB is 1
    BitReader bitReader2(testData2, 1);

    int symbol2 = decoder.DecodeSymbol(bitReader2);
    if (symbol2 != 1)
    {
        std::cout << "ERROR: Expected symbol 1, got " << symbol2 << std::endl;
        return false;
    }

    std::cout << "SUCCESS: JPEG Huffman decoding test passed!" << std::endl;
    return true;
}

bool TestJPEGCoefficientDecoder()
{
    std::cout << "\n=== Testing JPEG Coefficient Decoder ===" << std::endl;

    // Create simple DC and AC Huffman tables for testing

    // DC table: simple 1-bit codes for categories 0 and 1
    uint8_t dcCodeLengths[16] = {
        2, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t dcSymbols[2] = {0x00, 0x01}; // Categories 0 and 1

    // AC table: simple codes for EOB and a basic AC symbol
    uint8_t acCodeLengths[16] = {
        2, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t acSymbols[2] = {0x00, 0x11}; // EOB and run=1, category=1

    JPEGHuffmanDecoder dcDecoder, acDecoder;

    if (!dcDecoder.BuildTree(dcCodeLengths, dcSymbols, 2))
    {
        std::cout << "ERROR: Failed to build DC Huffman tree" << std::endl;
        return false;
    }

    if (!acDecoder.BuildTree(acCodeLengths, acSymbols, 2))
    {
        std::cout << "ERROR: Failed to build AC Huffman tree" << std::endl;
        return false;
    }

    JPEGCoefficientDecoder coeffDecoder;
    coeffDecoder.SetHuffmanDecoders(&dcDecoder, &acDecoder);

    // Test data:
    // DC category 1 (code 1, 1 bit) = bit 1
    // DC value 1 (1 bit for category 1) = bit 1
    // AC EOB (code 0, 1 bit) = bit 0
    // Total: 1 1 0 = LSB order: 0b00000011 (3 bits: 1,1,0)
    uint8_t testData[] = {0x03};
    BitReader bitReader(testData, 1);

    int16_t coefficients[64];

    if (!coeffDecoder.DecodeBlock(bitReader, coefficients, 0))
    {
        std::cout << "ERROR: Failed to decode coefficient block" << std::endl;
        std::cout << "Last error: " << coeffDecoder.GetLastError() << std::endl;
        return false;
    }

    // Check DC coefficient (should be 1 due to DC prediction)
    if (coefficients[0] != 1)
    {
        std::cout << "ERROR: Expected DC coefficient 1, got " << coefficients[0] << std::endl;
        return false;
    }

    // Check that AC coefficients are zero (EOB was decoded)
    bool allACZero = true;
    for (int i = 1; i < 64; ++i)
    {
        if (coefficients[i] != 0)
        {
            allACZero = false;
            break;
        }
    }

    if (!allACZero)
    {
        std::cout << "ERROR: AC coefficients should be zero after EOB" << std::endl;
        return false;
    }

    std::cout << "SUCCESS: JPEG coefficient decoder test passed!" << std::endl;
    return true;
}

bool TestDCPrediction()
{
    std::cout << "\n=== Testing DC Prediction ===" << std::endl;

    // Create a simple DC table
    uint8_t dcCodeLengths[16] = {
        1, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t dcSymbols[2] = {0x00, 0x01}; // Categories 0 and 1

    uint8_t acCodeLengths[16] = {
        1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t acSymbols[1] = {0x00}; // Just EOB

    JPEGHuffmanDecoder dcDecoder, acDecoder;
    dcDecoder.BuildTree(dcCodeLengths, dcSymbols, 2);
    acDecoder.BuildTree(acCodeLengths, acSymbols, 1);

    JPEGCoefficientDecoder coeffDecoder;
    coeffDecoder.SetHuffmanDecoders(&dcDecoder, &acDecoder);

    // Test DC prediction: decode two blocks with DC differences
    // Block 1: DC category 1, DC diff +1, EOB
    // Block 2: DC category 1, DC diff +1, EOB
    // Expected DC values: 1, 2 (cumulative)

    // Test data for two blocks (need 6 bits total):
    // Block 1: DC category 1 (code 1), DC diff +1 (1 bit), EOB (code 0)
    // Block 2: DC category 1 (code 1), DC diff +1 (1 bit), EOB (code 0)
    // Pattern: 1 1 0 1 1 0 = LSB order: 0b00110011 (6 bits in one byte)
    uint8_t testData[] = {0x33}; // 0b00110011
    BitReader bitReader(testData, 1);

    int16_t coefficients1[64], coefficients2[64];

    // Decode first block
    if (!coeffDecoder.DecodeBlock(bitReader, coefficients1, 0))
    {
        std::cout << "ERROR: Failed to decode first block" << std::endl;
        std::cout << "Last error: " << coeffDecoder.GetLastError() << std::endl;
        return false;
    }

    // The bit reader should automatically continue to the next block
    // Decode second block (continuing from the same bit stream)
    if (!coeffDecoder.DecodeBlock(bitReader, coefficients2, 0))
    {
        std::cout << "ERROR: Failed to decode second block" << std::endl;
        std::cout << "Last error: " << coeffDecoder.GetLastError() << std::endl;
        return false;
    }

    // Check DC prediction
    if (coefficients1[0] != 1)
    {
        std::cout << "ERROR: First block DC should be 1, got " << coefficients1[0] << std::endl;
        return false;
    }

    if (coefficients2[0] != 2)
    {
        std::cout << "ERROR: Second block DC should be 2, got " << coefficients2[0] << std::endl;
        return false;
    }

    std::cout << "SUCCESS: DC prediction test passed!" << std::endl;
    return true;
}

int main()
{
    std::cout << "Starting JPEG Huffman Decoder Tests..." << std::endl;

    bool allTestsPassed = true;

    allTestsPassed &= TestJPEGHuffmanTreeConstruction();
    allTestsPassed &= TestJPEGHuffmanDecoding();
    allTestsPassed &= TestJPEGCoefficientDecoder();
    allTestsPassed &= TestDCPrediction();

    std::cout << "\n=== Test Results ===" << std::endl;
    if (allTestsPassed)
    {
        std::cout << "🎉 ALL JPEG HUFFMAN TESTS PASSED!" << std::endl;
        std::cout << "JPEG Huffman decoding is working correctly." << std::endl;
        std::cout << "Ready to implement dequantization and IDCT." << std::endl;
    }
    else
    {
        std::cout << "❌ SOME TESTS FAILED! Please check the implementation." << std::endl;
    }

    return allTestsPassed ? 0 : 1;
}
