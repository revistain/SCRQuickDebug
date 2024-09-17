#include "encoding.h"

/*
bool is_utf8(const std::string& str) {
    size_t i = 0;
    size_t len = str.length();

    while (i < len) {
        unsigned char c = str[i];

        if (c <= 0x7F) {
            // ASCII character
            i++;
        }
        else if ((c >= 0xC2 && c <= 0xDF) && i + 1 < len && (str[i + 1] & 0xC0) == 0x80) {
            // 2-byte UTF-8 sequence
            i += 2;
        }
        else if ((c >= 0xE0 && c <= 0xEF) && i + 2 < len && (str[i + 1] & 0xC0) == 0x80 && (str[i + 2] & 0xC0) == 0x80) {
            // 3-byte UTF-8 sequence
            i += 3;
        }
        else if ((c >= 0xF0 && c <= 0xF4) && i + 3 < len && (str[i + 1] & 0xC0) == 0x80 && (str[i + 2] & 0xC0) == 0x80 && (str[i + 3] & 0xC0) == 0x80) {
            // 4-byte UTF-8 sequence
            i += 4;
        }
        else {
            // Invalid UTF-8 byte sequence
            return false;
        }
    }
    return true;
}

// Convert CP949 to UTF-8 (Windows)
#ifdef _WIN32
std::string convert_cp949_to_utf8(const std::string& input) {
    // Convert input from CP949 to UTF-16
    int wide_size = MultiByteToWideChar(949, 0, input.c_str(), -1, NULL, 0);
    std::wstring wide_str(wide_size, 0);
    MultiByteToWideChar(949, 0, input.c_str(), -1, &wide_str[0], wide_size);

    // Convert from UTF-16 to UTF-8
    int utf8_size = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, NULL, 0, NULL, NULL);
    std::string utf8_str(utf8_size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, &utf8_str[0], utf8_size, NULL, NULL);

    return utf8_str;
}
#endif
*/
SCText::SCText(std::string&& _text) {
    encodeType = ENCODING::UTF_8;
    text = std::move(_text);
    /*
    if (is_utf8(_text)) {
        encodeType = ENCODING::UTF_8;
        text = std::move(_text);
    }
    else {
        encodeType = ENCODING::CP949;
        text = convert_cp949_to_utf8(_text);
    }
    */
}

/*
std::ostream& operator<<(std::ostream& os, const SCText& scText) {
    os << scText.text;
    return os;
}
*/