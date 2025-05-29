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

uint8_t S_prime[256];

uint8_t rotate_right(uint8_t byte, int p) {
    return (byte >> p) | (byte << (8 - p));
}

const string input_bits = "0000000100100011010001010110011110001001101010111100110111101111";
const int p = 5;

void build_S_prime(int p) {
    for (int x = 0; x < 256; ++x) {
        uint8_t x1 = (x >> 4) & 0x0F;
        uint8_t x2 = x & 0x0F;
        uint8_t a = (S[x1] << 4) | S[x2];
        S_prime[x] = rotate_right(a, p);
    }
}

int main() {
    
    build_S_prime(p);

    string result;

    for (int i = 0; i < 64; i += 8) {
        bitset<8> block(input_bits.substr(i, 8));
        uint8_t input_byte = block.to_ulong();
        uint8_t output_byte = S_prime[input_byte];
        result += bitset<8>(output_byte).to_string();
    }

    cout << "Output: " << result << endl;
    cout << "S' size: " << sizeof(S_prime) << " bytes" << endl;

    return 0;
}
