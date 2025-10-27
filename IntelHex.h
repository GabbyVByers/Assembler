#pragma once

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

static std::string make_record(uint16_t addr, uint16_t word) {
    uint8_t bytes[] = {
        0x02,                     // 2 bytes
        uint8_t(addr >> 8),       // address high
        uint8_t(addr & 0xFF),     // address low
        0x00,                     // record type
        uint8_t(word >> 8),       // data high
        uint8_t(word & 0xFF)      // data low
    };
    uint16_t sum = 0;
    for (uint8_t b : bytes) sum += b;
    uint8_t checksum = (uint8_t)((0x100 - (sum & 0xFF)) & 0xFF);

    std::ostringstream out;
    out << ':' << std::uppercase << std::hex << std::setfill('0')
        << std::setw(2) << int(bytes[0])
        << std::setw(4) << addr
        << std::setw(2) << int(bytes[3])
        << std::setw(4) << word
        << std::setw(2) << int(checksum);
    return out.str();
}

static void build_intel_hex(std::string text, std::string hex) {
    std::ifstream fin(text);
    std::ofstream fout(hex);
    std::string line;
    uint16_t addr = 0;

    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        uint16_t word = std::stoul(line, nullptr, 16);
        fout << make_record(addr, word) << '\n';
        addr++;
    }
    fout << ":00000001FF\n";
}

