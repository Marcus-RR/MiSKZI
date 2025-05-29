#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <random>

const size_t BLOCK_SIZE = 8;               // 64 бита
const size_t KEY_SIZE = 7;                 // 56 бит
const size_t MAX_ENCRYPT_SIZE = 20480;     // 20 КБ
const size_t WARNING_ENCRYPT_SIZE = 10240; // 10 КБ

// S-блоки Магма (из ГОСТ Р 34.12-2015, таблица 1, тестовый набор)
const uint8_t SBOX[8][16] = {
    { 12, 4, 6, 2, 10, 5, 11, 9, 14, 8, 13, 15, 3, 7, 1, 0 },
    { 15, 12, 8, 2, 10, 0, 4, 13, 14, 9, 1, 7, 6, 3, 11, 5 },
    { 6, 8, 2, 3, 9, 10, 5, 12, 1, 14, 4, 7, 0, 15, 13, 11 },
    { 12, 7, 2, 1, 6, 0, 8, 13, 3, 15, 9, 10, 4, 5, 14, 11 },
    { 7, 15, 5, 10, 8, 1, 6, 13, 0, 9, 3, 14, 11, 4, 2, 12 },
    { 5, 13, 15, 6, 9, 2, 1, 8, 0, 14, 10, 4, 7, 3, 11, 12 },
    { 8, 14, 7, 11, 0, 10, 9, 1, 13, 3, 15, 6, 2, 5, 12, 4 },
    { 9, 6, 3, 15, 1, 13, 14, 0, 11, 2, 8, 5, 12, 10, 4, 7 }
};

// Генерация 56-битного ключа
void generate_key(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 255);
    for (size_t i = 0; i < KEY_SIZE; ++i) {
        uint8_t byte = dist(gen);
        out.write(reinterpret_cast<char*>(&byte), 1);
    }
    std::cout << "Ключ сгенерирован: " << filename << "\n";
}

// Чтение ключа из файла
std::vector<uint8_t> read_key(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Ошибка чтения ключа.");
    std::vector<uint8_t> key(KEY_SIZE);
    file.read(reinterpret_cast<char*>(key.data()), KEY_SIZE);
    if (file.gcount() != KEY_SIZE) throw std::runtime_error("Ключ должен быть 56 бит.");
    return key;
}

// Разворачивание ключа до 256 бит (32 x 8 = 256)
std::vector<uint32_t> expand_key(const std::vector<uint8_t>& key) {
    std::vector<uint8_t> full_key;
    for (int i = 0; i < 4; ++i)
        full_key.insert(full_key.end(), key.begin(), key.end());
    full_key.insert(full_key.end(), key.begin(), key.begin() + 4);

    std::vector<uint32_t> keys;
    for (int i = 0; i < 32; ++i) {
        size_t offset = i * 4;
        uint32_t k = (full_key[offset] << 24) | (full_key[offset + 1] << 16) |
                     (full_key[offset + 2] << 8) | full_key[offset + 3];
        keys.push_back(k);
    }
    return keys;
}

uint32_t apply_sbox(uint32_t val) {
    uint32_t res = 0;
    for (int i = 0; i < 8; ++i) {
        uint8_t nibble = (val >> (4 * i)) & 0xF;
        res |= SBOX[i][nibble] << (4 * i);
    }
    return res;
}

uint32_t magma_round(uint32_t data, uint32_t key) {
    uint32_t temp = (data + key) % 0x100000000;
    temp = apply_sbox(temp);
    return (temp << 11) | (temp >> (32 - 11));
}

void encrypt_block(uint8_t* block, const std::vector<uint32_t>& keys) {
    uint32_t n1 = (block[0] << 24) | (block[1] << 16) | (block[2] << 8) | block[3];
    uint32_t n2 = (block[4] << 24) | (block[5] << 16) | (block[6] << 8) | block[7];
    for (int i = 0; i < 31; ++i) {
        uint32_t temp = n1;
        n1 = n2 ^ magma_round(n1, keys[i]);
        n2 = temp;
    }
    n2 ^= magma_round(n1, keys[31]);
    block[0] = n1 >> 24; block[1] = n1 >> 16; block[2] = n1 >> 8; block[3] = n1;
    block[4] = n2 >> 24; block[5] = n2 >> 16; block[6] = n2 >> 8; block[7] = n2;
}

void decrypt_block(uint8_t* block, const std::vector<uint32_t>& keys) {
    uint32_t n1 = (block[0] << 24) | (block[1] << 16) | (block[2] << 8) | block[3];
    uint32_t n2 = (block[4] << 24) | (block[5] << 16) | (block[6] << 8) | block[7];
    for (int i = 0; i < 31; ++i) {
        uint32_t temp = n1;
        n1 = n2 ^ magma_round(n1, keys[31 - i]);
        n2 = temp;
    }
    n2 ^= magma_round(n1, keys[0]);
    block[0] = n1 >> 24; block[1] = n1 >> 16; block[2] = n1 >> 8; block[3] = n1;
    block[4] = n2 >> 24; block[5] = n2 >> 16; block[6] = n2 >> 8; block[7] = n2;
}

void pad_data(std::vector<uint8_t>& data) {
    size_t pad_len = BLOCK_SIZE - (data.size() % BLOCK_SIZE);
    data.push_back(0x80);
    for (size_t i = 1; i < pad_len; ++i)
        data.push_back(0x00);
}

void unpad_data(std::vector<uint8_t>& data) {
    while (!data.empty() && data.back() == 0x00) data.pop_back();
    if (!data.empty() && data.back() == 0x80) data.pop_back();
}

void encrypt_file(const std::string& in_file, const std::string& out_file, const std::vector<uint32_t>& keys) {
    std::ifstream in(in_file, std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), {});
    
    if (data.size() > MAX_ENCRYPT_SIZE) throw std::runtime_error("Превышен лимит 20КБ!");
    if (data.size() > WARNING_ENCRYPT_SIZE)
        std::cerr << "Предупреждение: превышен 10КБ, рекомендуется сменить ключ.\n";

    pad_data(data);
    for (size_t i = 0; i < data.size(); i += BLOCK_SIZE)
        encrypt_block(&data[i], keys);

    std::ofstream out(out_file, std::ios::binary);
    out.write(reinterpret_cast<char*>(data.data()), data.size());
}

void decrypt_file(const std::string& in_file, const std::string& out_file, const std::vector<uint32_t>& keys) {
    std::ifstream in(in_file, std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), {});

    for (size_t i = 0; i < data.size(); i += BLOCK_SIZE)
        decrypt_block(&data[i], keys);

    unpad_data(data);

    std::ofstream out(out_file, std::ios::binary);
    out.write(reinterpret_cast<char*>(data.data()), data.size());
}

void modify_encrypted_file() {
    std::string file = "output.enc";
    std::fstream f(file, std::ios::in | std::ios::out | std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open output.enc");

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), {});
    f.close();

    char option;
    std::cout << "Выберите операцию:\n";
    std::cout << "  1. Удалить 1 байт данных\n";
    std::cout << "  2. Удалить хвост, не кратный 64 битам\n";
    std::cout << "  3. Удалить 1 блок (8 байт)\n";
    std::cout << "  4. Добавить блок (8 байт)\n";
    std::cout << "  5. Переставить два блока\n";
    std::cout << "Ваш выбор: ";
    std::cin >> option;

    switch (option) {
    	case '1': {
    		if (!data.empty()) data.erase(data.begin());
    		break;
		}
        case '2': {
            size_t rem = data.size() % BLOCK_SIZE;
            if (rem != 0) data.erase(data.end() - rem, data.end());
            break;
        }
        case '3': if (data.size() >= BLOCK_SIZE)
                      data.erase(data.end() - BLOCK_SIZE, data.end());
                  break;
        case '4': {
            std::vector<uint8_t> block = {'B','L','O','C','K','0','0','1'};
            data.insert(data.end(), block.begin(), block.end());
            break;
        }
        case '5': {
            if (data.size() >= 2 * BLOCK_SIZE) {
                size_t idx1 = 0, idx2 = BLOCK_SIZE;
                std::swap_ranges(data.begin() + idx1, data.begin() + idx1 + BLOCK_SIZE,
                                 data.begin() + idx2);
            }
            break;
        }
        default:
            std::cerr << "Неверный выбор.\n";
            return;
    }

    std::ofstream fout(file, std::ios::binary | std::ios::trunc);
    fout.write(reinterpret_cast<char*>(data.data()), data.size());
    std::cout << "Файл успешно изменён.\n";
}

int main() {
    const std::string key_file = "key.key";
    const std::string input = "input.txt";
    const std::string encrypted = "output.enc";
    const std::string decrypted = "output.txt";

    try {
        if (!std::ifstream(key_file)) generate_key(key_file);

        auto key = read_key(key_file);
        auto keys = expand_key(key);

        while (true) {
            char choice;
            std::cout << "\nВыберите режим:\n";
            std::cout << "  [h] Шифрование (input.txt → output.enc)\n";
            std::cout << "  [r] Расшифровка (output.enc → output.txt)\n";
            std::cout << "  [g] Модификация выходного файла\n";
            std::cout << "  [q] Выход\n";
            std::cout << "Ваш выбор: ";
            std::cin >> choice;

            if (choice == 'h' || choice == 'H') {
                encrypt_file(input, encrypted, keys);
                std::cout << "Шифрование завершено: " << encrypted << "\n";
            } else if (choice == 'r' || choice == 'R') {
                decrypt_file(encrypted, decrypted, keys);
                std::cout << "Расшифровка завершена: " << decrypted << "\n";
            } else if (choice == 'g' || choice == 'G') {
                modify_encrypted_file();
            } else if (choice == 'q' || choice == 'Q') {
                std::cout << "Завершение программы.\n";
                break;
            } else {
                std::cerr << "Неверный выбор. Используйте 'h', 'r', 'g' или 'q'.\n";
            }
        }

    } catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << "\n";
    }

    return 0;
}
