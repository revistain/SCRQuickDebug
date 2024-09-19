//#define _CRT_SECURE_NO_WARNINGS
//#ifndef SECTION_H
//#define SECTION_H
//#include <ostream>
//#include <stdint.h>
//#include <vector>
//#include <string>
//#include <cstring>
//#include <algorithm>
//#include <iostream>
//#include <sstream>
//#include <iomanip>
//#include <stdexcept>
//#include "encoding.h"
//
//class Section {
//private:
//    char type[5];
//    uint32_t size;
//    std::vector<uint8_t> data;
//    std::vector<SCText> stringTable;
//
//    Section operator=(Section&) = delete;
//    bool operator==(const Section& section) const { return (strncmp(type, section.type, 4) == 0); }
//public:
//    explicit Section(const char* _type, uint32_t _size, const std::vector<uint8_t>& chk, uint32_t index);
//    explicit Section(const char* _type, uint32_t _size) : size(_size) { strncpy(type, _type, 4); };
//    void feedSection(std::vector<uint8_t>& new_section_data, uint32_t _size);
//    void extractSTRxData();
//    bool checkName(const char* name) const { return (strncmp(type, name, 4) == 0); }
//    std::vector<uint8_t>& getData() { return data; }
//
//    uint8_t bread(uint32_t offset);
//    uint16_t wread(uint32_t offset);
//    uint32_t dwread(uint32_t offset);
//
//
//    const std::string repr() const;
//    static Section nullSection;
//};
//#endif
