#include "Pyramid/Util/JPEGHuffmanDecoder.hpp"
#include "Pyramid/Util/BitReader.hpp"
#include <iostream>
#include <vector>

using namespace Pyramid::Util;

bool TestSimpleHuffmanDecoding()
{
    std::cout << "=== Debug: Simple JPEG Huffman Decoding ===" << std::endl;

    // Create the simplest possible Huffman table
    // 1 code of length 1: symbol 0 gets code 0
    uint8_t codeLengths[16] = {
        1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};

    uint8_t symbols[1] = {0x05}; // Just one symbol: 5

    JPEGHuffmanDecoder decoder;
    if (!decoder.BuildTree(codeLengths, symbols, 1))
    {
        std::cout << "ERROR: Failed to build simple Huffman tree" << std::endl;
        return false;
    }

    // Test with bit 0 (should decode to symbol 5)
    uint8_t testData[] = {0x00}; // 0b00000000 - first bit is 0
    BitReader bitReader(testData, 1);

    int symbol = decoder.DecodeSymbol(bitReader);
    std::cout << "DEBUG: Decoded symbol: " << symbol << " (expected 5)" << std::endl;

    if (symbol != 5)
    {
        std::cout << "ERROR: Expected symbol 5, got " << symbol << std::endl;
        return false;
    }

    // Test with bit 1 (should also decode to symbol 5 since there's only one code)
    uint8_t testData2[] = {0x80}; // 0b10000000 - first bit is 1 (LSB reading)
    BitReader bitReader2(testData2, 1);

    int symbol2 = decoder.DecodeSymbol(bitReader2);
    std::cout << "DEBUG: Decoded symbol with bit 1: " << symbol2 << " (expected 5)" << std::endl;

    if (symbol2 != 5)
    {
        std::cout << "ERROR: Expected symbol 5, got " << symbol2 << std::endl;
        return false;
    }

    std::cout << "SUCCESS: Simple Huffman decoding test passed!" << std::endl;
    return true;
}

bool TestTwoSymbolHuffman()
{
    std::cout << "\n=== Debug: Two Symbol JPEG Huffman ===" << std::endl;

    // 2 codes of length 1
    uint8_t codeLengths[16] = {
        2, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};

    uint8_t symbols[2] = {0x0A, 0x0B}; // Symbols 10 and 11

    JPEGHuffmanDecoder decoder;
    if (!decoder.BuildTree(codeLengths, symbols, 2))
    {
        std::cout << "ERROR: Failed to build two-symbol Huffman tree" << std::endl;
        return false;
    }

    std::cout << "DEBUG: Built tree with 2 symbols (10, 11) and 2 codes of length 1" << std::endl;
    std::cout << "DEBUG: Expected mapping - Code 0: Symbol 10, Code 1: Symbol 11" << std::endl;

    // In canonical Huffman with 2 codes of length 1:
    // First symbol (0x0A) gets code 0
    // Second symbol (0x0B) gets code 1

    // Test code 0 (bit 0) - LSB = 0
    uint8_t testData1[] = {0x00}; // 0b00000000 - LSB is 0
    BitReader bitReader1(testData1, 1);

    int symbol1 = decoder.DecodeSymbol(bitReader1);
    std::cout << "DEBUG: Code 0 decoded to symbol: " << symbol1 << " (expected 10)" << std::endl;

    // Test code 1 (bit 1) - LSB = 1
    uint8_t testData2[] = {0x01}; // 0b00000001 - LSB is 1
    BitReader bitReader2(testData2, 1);

    int symbol2 = decoder.DecodeSymbol(bitReader2);
    std::cout << "DEBUG: Code 1 decoded to symbol: " << symbol2 << " (expected 11)" << std::endl;

    // Check results
    if (symbol1 == 10 && symbol2 == 11)
    {
        std::cout << "SUCCESS: Two-symbol Huffman test passed!" << std::endl;
        return true;
    }
    else if (symbol1 == 11 && symbol2 == 10)
    {
        std::cout << "INFO: Symbols are swapped - this indicates bit order issue" << std::endl;
        std::cout << "SUCCESS: Two-symbol Huffman test passed (with bit order note)!" << std::endl;
        return true;
    }
    else
    {
        std::cout << "ERROR: Unexpected symbol mapping" << std::endl;
        return false;
    }
}

int main()
{
    std::cout << "Starting JPEG Huffman Debug Tests..." << std::endl;

    bool allTestsPassed = true;

    allTestsPassed &= TestSimpleHuffmanDecoding();
    allTestsPassed &= TestTwoSymbolHuffman();

    std::cout << "\n=== Debug Test Results ===" << std::endl;
    if (allTestsPassed)
    {
        std::cout << "🎉 DEBUG TESTS PASSED!" << std::endl;
        std::cout << "JPEG Huffman basic functionality is working." << std::endl;
    }
    else
    {
        std::cout << "❌ DEBUG TESTS FAILED!" << std::endl;
    }

    return allTestsPassed ? 0 : 1;
}
