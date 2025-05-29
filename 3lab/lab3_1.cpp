#include <iostream>
#include <vector>
#include <bitset>

using namespace std;

const uint8_t S[16] = {
    0xB, 0x3, 0x5, 0x8,
    0x2, 0xF, 0xA, 0xD,
    0xE, 0x1, 0x7, 0x4,
    0xC, 0x9, 0x6, 0x0
};

const string input_bits = "0000000100100011010001010110011110001001101010111100110111101111";

uint8_t substitute(uint8_t nibble) {
    return S[nibble];
}

int main() {
    string result;
    for (int i = 0; i < 64; i += 4) {
        bitset<4> nibble(input_bits.substr(i, 4));
        uint8_t substituted = substitute(nibble.to_ulong());
        bitset<4> substituted_bits(substituted);
        result += substituted_bits.to_string();
    }

    cout << "Input:  " << input_bits << endl;
    cout << "Output: " << result << endl;

    return 0;
}
