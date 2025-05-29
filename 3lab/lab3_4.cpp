#include <iostream>
#include <bitset>

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

const string input_bits = "0000000100100011010001010110011110001001101010111100110111101111";
const int p = 5;

int main() {

    string result;

    for (int i = 0; i < 64; i += 8) {
        bitset<8> block(input_bits.substr(i, 8));
        uint8_t byte = block.to_ulong();

        uint8_t x1 = (byte >> 4) & 0x0F;
        uint8_t x2 = byte & 0x0F;

        uint8_t s1 = S[x1];
        uint8_t s2 = S[x2];

        uint8_t a = (s1 << 4) | s2;
        uint8_t b = rotate_right(a, p);

        result += bitset<8>(b).to_string();
    }

    cout << "Input:  " << input_bits << endl;
    cout << "Output: " << result << endl;

    return 0;
}
