#include <iostream>
#include <bitset>
#include <vector>

using namespace std;

const uint8_t S[16] = {
    0xB, 0x3, 0x5, 0x8,
    0x2, 0xF, 0xA, 0xD,
    0xE, 0x1, 0x7, 0x4,
    0xC, 0x9, 0x6, 0x0
};

uint8_t S_star[256];

const string input_bits = "0000000100100011010001010110011110001001101010111100110111101111";

void build_S_star() {
    for (int i = 0; i < 256; ++i) {
        uint8_t x1 = (i >> 4) & 0x0F;
        uint8_t x2 = i & 0x0F;
        S_star[i] = (S[x1] << 4) | S[x2];
    }
}

int main() {
    build_S_star();
    string result;

    for (int i = 0; i < 64; i += 8) {
        bitset<8> byte(input_bits.substr(i, 8));
        uint8_t substituted = S_star[byte.to_ulong()];
        bitset<8> substituted_bits(substituted);
        result += substituted_bits.to_string();
    }

    cout << "Output: " << result << endl;
    cout << "S* size: " << sizeof(S_star) << " bytes" << endl;
    return 0;
}
