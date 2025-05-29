#include <iostream>
#include <bitset>

using namespace std;

const string input_bits = "0000000100100011010001010110011110001001101010111100110111101111";
const int p = 5;

uint8_t rotate_right(uint8_t byte, int p) {
    return (byte >> p) | (byte << (8 - p));
}

int main() {
    string result;

    for (int i = 0; i < 64; i += 8) {
        bitset<8> block(input_bits.substr(i, 8));
        uint8_t rotated = rotate_right(block.to_ulong(), p);
        bitset<8> rotated_bits(rotated);
        result += rotated_bits.to_string();
    }

    cout << "Input:  " << input_bits << endl;
    cout << "Output: " << result << endl;

    return 0;
}
