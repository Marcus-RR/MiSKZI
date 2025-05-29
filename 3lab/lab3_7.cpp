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

uint8_t S_prime[256];

uint8_t rotate_right(uint8_t byte, int p) {
    return (byte >> p) | (byte << (8 - p));
}

const string input_file = "3_5inp.txt";
const string output_file = "3_7out.txt";
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

    ifstream inFile(input_file, ios::binary);
    ofstream outFile(output_file, ios::binary);

    char byte;
    while (inFile.get(byte)) {
        uint8_t input_byte = static_cast<uint8_t>(byte);
        uint8_t output_byte = S_prime[input_byte];
        outFile.put(static_cast<char>(output_byte));
    }

    inFile.close();
    outFile.close();

    cout << "S’ size: " << sizeof(S_prime) << " bytes" << endl;
    return 0;
}
