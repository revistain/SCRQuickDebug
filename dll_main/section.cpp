#include "section.h"

Section Section::nullSection("NULL", 0);
Section::Section(const char* _type, uint32_t _size, const std::vector<uint8_t>& chk, uint32_t index) : size(_size), data(chk.begin() + index, chk.begin() + index + _size) {
    std::strncpy(type, _type, 4);
    type[4] = '\0';
}

void Section::extractSTRxData() {
    const uint32_t table_legnth = *reinterpret_cast<const int32_t*>(&data[0]);
    for (uint32_t i = 1; i <= 20; i++) {
        const uint32_t offset = *reinterpret_cast<const int32_t*>(&data[i * 4]);
        stringTable.emplace_back(std::string(data.begin() + offset, std::find(data.begin() + offset, data.end(), static_cast<uint8_t>('\0'))));
        std::cout << "STRx index " << i << ": " << stringTable[stringTable.size() - 1].text << std::endl;
    }
}

// later section must call previous section
void Section::feedSection(std::vector<uint8_t>& new_section_data, uint32_t _size) {
    if (size < _size) {
        size = _size;
        data.resize(_size);
    }
    if (data.size() >= new_section_data.size()) std::move(new_section_data.begin(), new_section_data.end(), data.begin());
    else std::cerr << "Error: Data vector is not large enough." << std::endl;
}

const std::string Section::repr() const {
    std::stringstream size_ss;
    size_ss << std::hex << std::setw(4) << std::setfill('0') << size;
    std::string size_str = size_ss.str();

    std::string result = "\tNAME: " + std::string(type) + "\t SIZE: 0x" + size_str;
    return result;
}

uint8_t Section::bread(uint32_t offset) {
    if (offset >= size) throw std::runtime_error("Section::bread memory out of bound");
    return data[offset];
}
uint16_t Section::wread(uint32_t offset) {
    if (offset >= size + 1) throw std::runtime_error("Section::wread memory out of bound");
    return data[offset] + (data[offset + 1] << 8);
}
uint32_t Section::dwread(uint32_t offset) {
    if (offset >= size + 3) throw std::runtime_error("Section::dwread memory out of bound");
    return data[offset] + (data[offset + 1] << 8) + (data[offset + 2] << 16) + (data[offset + 3] << 24);
}