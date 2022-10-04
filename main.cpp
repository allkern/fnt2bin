#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#include <iomanip>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "log.hpp"

static const char* base64_enc_table = "ABCDEFGHIJKLMNOP"
                                      "QRSTUVWXYZabcdef" 
                                      "ghijklmnopqrstuv"
                                      "wxyz0123456789+/";

static const char* base64_dec_table = "\x90\x24\xe2\x65\x5d\x01\x00\x00"
                                      "\x00\x61\xd9\x65\x5d\x01\x00\x00"
                                      "\x00\x00\x00\x00\x00\x00\x00\x00"
                                      "\x00\x00\x00\x00\x00\x00\x00\x00"
                                      "\x00\x00\x00\x00\x00\x00\x00\x00"
                                      "\xb3\x00\x00\x3e\x69\x7c\x00\x3f"
                                      "\x34\x35\x36\x37\x38\x39\x3a\x3b"
                                      "\x3c\x3d\xd9\x65\x5d\x01\x00\x00"
                                      "\x00\x00\x01\x02\x03\x04\x05\x06"
                                      "\x07\x08\x09\x0a\x0b\x0c\x0d\x0e"
                                      "\x0f\x10\x11\x12\x13\x14\x15\x16"
                                      "\x17\x18\x19\x65\x5d\x01\x00\x00"
                                      "\x00\x1a\x1b\x1c\x1d\x1e\x1f\x20"
                                      "\x21\x22\x23\x24\x25\x26\x27\x28"
                                      "\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30"
                                      "\x31\x32\x33\x00\x00\x00\x00\x00";

std::string encode_base64(std::string str) {
    uint32_t buf = 0;

    std::string b64;
    
    for (int i = 0; i < str.size(); i += 3) {
        buf  = str[i + 0] << 16;
        buf |= str[i + 1] << 8;
        buf |= str[i + 2] << 0;

        b64.push_back(base64_enc_table[(buf >> 18) & 0x3f]);
        b64.push_back(base64_enc_table[(buf >> 12) & 0x3f]);
        b64.push_back(base64_enc_table[(buf >> 6 ) & 0x3f]);
        b64.push_back(base64_enc_table[(buf >> 0 ) & 0x3f]);
    }

    return b64;
}

std::string decode_base64(std::string str) {
    std::string data;

    uint32_t buf;

    for (int i = 0; i < str.size(); i += 4) {
        buf  = (str[i + 0] == '=') ? 0 : base64_dec_table[str[i + 0]] << 18; 
        buf |= (str[i + 1] == '=') ? 0 : base64_dec_table[str[i + 1]] << 12; 
        buf |= (str[i + 2] == '=') ? 0 : base64_dec_table[str[i + 2]] << 6; 
        buf |= (str[i + 3] == '=') ? 0 : base64_dec_table[str[i + 3]] << 0;

        for (int i = 0; i < 3; i++)
            data.push_back(buf >> ((2 - i) << 3));
    }

    return data;
}

int main(int argc, const char* argv[]) {
    _log::init("fnt2bin");

    if (!argv[1]) {
        _log(error, "No input files specified");

        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);

    if (!(file.is_open() && file.good())) {
        _log(error, "Couldn't open file \"%s\"", argv[1]);

        return 1;
    }

    char current;

    size_t len;
    std::string data_b64;
    std::string data_dec;

    int w = 0, h = 0, f = 0;
    uint32_t* image = nullptr;

    current = file.get();

    while (!file.eof()) {
        if (!std::isalpha(current))
            current = file.get();

        std::string text;
        
        while (std::isalpha(current)) {
            text.push_back(current);

            current = file.get();
        }

        if (text.size()) {
            if (text == "datalen") {
                std::string len_str;

                current = file.get();

                while (std::isdigit(current)) {
                    len_str.push_back(current);

                    current = file.get();
                }

                len = std::stoul(len_str);
            }

            if (text == "data") {
                current = file.get();

                size_t counter = len;

                while (counter--) {
                    data_b64.push_back((unsigned char)current);

                    current = file.get();
                }
                
                data_dec = decode_base64(data_b64);

                image = (uint32_t*)stbi_load_from_memory((uint8_t*)data_dec.data(), data_dec.size(), &w, &h, &f, 4);

                if (!image) {
                    _log(error, "STBI couldn't load image");

                    return 1;
                }
            }
        }

        text.clear();
    }

    bool make_png = false;

    if (make_png) {
        std::ofstream png("font.png", std::ios::binary);

        png.write((char*)data_dec.data(), data_dec.size());

        png.close();
    }

    std::ofstream font("font.bin", std::ios::binary);

    for (int y = 0; y < 6; y++) {
        for (int x = 0; x < 7; x++) {
            for (int cy = 0; cy < 8; cy++) {
                uint8_t row = 0;

                for (int cx = 0; cx < 8; cx++) {
                    if (image[(cx + (x * 8)) + ((cy + (y * 8)) * w)] >> 24)
                        row |= 1 << (7 - cx);
                }

                font.write((char*)&row, 1);
            }
        }
    }

    return 0;
}