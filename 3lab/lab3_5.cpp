#include <iostream>
#include <fstream>
#include <cstdint>

using namespace std;

const uint8_t S[16] = {
    0xB, 0x3, 0x5, 0x8,
    0x2, 0xF, 0xA, 0xD,
    0xE, 0x1, 0x7, 0x4,
    0xC, 0x9, 0x6, 0x0
};

uint8_t rotate_right(uint8_t byte, int p) {
    return (byte >> p) | (byte << (8 - p));
}

const string input_filename = "3_5inp.txt";
const string output_filename = "3_5out.txt";
const int p = 5;

uint8_t transform_byte(uint8_t byte) {
    uint8_t x1 = (byte >> 4) & 0x0F;
    uint8_t x2 = byte & 0x0F;

    uint8_t s1 = S[x1];
    uint8_t s2 = S[x2];

    uint8_t combined = (s1 << 4) | s2;

    return rotate_right(combined, p);
}

int main() {

    ifstream inFile(input_filename, ios::binary);
    ofstream outFile(output_filename, ios::binary);
    
    char byte;
    while (inFile.get(byte)) {
        uint8_t input_byte = static_cast<uint8_t>(byte);
        uint8_t output_byte = transform_byte(input_byte);
        outFile.put(static_cast<char>(output_byte));
    }

    inFile.close();
    outFile.close();

    return 0;
}
