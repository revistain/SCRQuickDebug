#include "mpq.h"
void MPQRead::open(const char* map_name) {
    if (!SFileOpenArchive((LPCSTR)map_name, 0, 0, &hMPQ)) {
        std::cerr << "cannot open map file\n";
        throw std::runtime_error("stormlib::cannot open map file");
    }

    std::cerr << "opened map file " << map_name << "\n";
}
#include <fstream>
void MPQRead::extractChk() {
    if (!SFileOpenFileEx(hMPQ, "staredit\\scenario.chk", 0, &hFILE)) throw std::runtime_error("stormlib::failed to open scneario.chk");
    fsize = SFileGetFileSize(hFILE, NULL);
    if (fsize == 0) throw std::runtime_error("stormlib::need to call MPQRead::Open first!");

    char buffer[4096];
    chk.resize(fsize);
    uint32_t remain_size = fsize;
    while (remain_size > 0) {
        uint32_t transfersize = std::min<uint32_t>((uint32_t)4096, remain_size);
        DWORD readbyte;
        if (!SFileReadFile(hFILE, buffer, transfersize, &readbyte, NULL)) throw std::runtime_error("stormlib::failed to read scneario.chk");
        if (readbyte != transfersize) throw std::runtime_error("stormlib::error while copying scneario.chk");
        std::memcpy(chk.data() + fsize - remain_size, buffer, readbyte);
        remain_size -= transfersize;
    }

    std::ofstream outputFile("test.chk", std::ios::binary);
    if (!outputFile) throw std::runtime_error("stormlib::error opening output file.");
    outputFile.write(reinterpret_cast<const char*>(chk.data()), chk.size());
    if (!outputFile) throw std::runtime_error("stormlib::error writing to output file.");

    SFileCloseFile(hFILE);
    SFileCloseArchive(hMPQ);
}

void MPQRead::parseChk() {
    char section_name[5]; section_name[4] = '\0';
    size_t chk_size = chk.size();

    for (size_t idx = 0; idx < chk_size;) {
        strncpy(section_name, reinterpret_cast<const char*>(&chk[idx]), 4);
        uint32_t section_size = *reinterpret_cast<const int32_t*>(&chk[idx + 4]);
        bool isDuplicated = false;
        for (auto& section : sections) {
            if (section.checkName(section_name)) {
                std::cout << "isDuplicated: " << section_name << std::endl;
                isDuplicated = true;
                std::vector<uint8_t> new_section_data(chk.begin() + static_cast<uint32_t>(idx + 8), chk.begin() + static_cast<uint32_t>(idx + 8) + section_size);
                section.feedSection(new_section_data, section_size);
                break;
            }
        }
        ;
        auto it = std::find(sectionNames.begin(), sectionNames.end(), section_name);
        if (!isDuplicated && it != sectionNames.end()) {
            sections.emplace_back(section_name, section_size, chk, static_cast<uint32_t>(idx + 8));
            // if(strncmp(section_name, "STRx", 4) == 0) sections[sections.size() - 1].extractSTRxData();
            sectionNames.erase(it);
        }

        if (section_size + 8 < chk_size) idx += (section_size + 8);
        else break;
    }

    // process sections
    std::pair<size_t, size_t> dimension;
    Section& dim_section = findSectionByName("DIM ");
    if (!dim_section.checkName("NULL")) {
        dimension.first = dim_section.wread(0) & 0xFFFF;
        dimension.second = dim_section.wread(2) & 0xFFFF;
        std::cerr << "\t================================================================\n";
        std::cerr << "\t===============   MAP DIMENSTION X: " << std::dec << dimension.first << " Y: " << dimension.second << "   ===============\n";
        std::cerr << "\t================================================================\n";
        if (dimension.first > 256 || dimension.second > 256) std::cerr << "WTF?\n";
    }
    Section& strx_section = findSectionByName("STRx");
    if (!strx_section.checkName("NULL")) strx_section.extractSTRxData();
    Section& mtxm_section = findSectionByName("MTXM");
    /*
    if (!mtxm_section.checkName("NULL")) {
        Minimap minimap = Minimap(mtxm_section.getData(), dimension.first, dimension.second);
        minimap.loadTileData();
        minimap.drawMinimap();
    }
    */


    // debug print strings
    for (auto& s : sections) {
        // std::cout << s.repr() << std::endl;
    }
}

Section& MPQRead::findSectionByName(const char* name) {
    if (sections.empty()) throw std::runtime_error("MPQRead::call this after scenario.chk extracted");
    for (auto& section : sections) {
        if (section.checkName(name)) {
            return section;
        }
    }
    return Section::nullSection;
}

bool extractMPQ() {
    try {
        MPQRead mr = MPQRead();
        mr.open("C:\\Users\\robin\\OneDrive\\¹®¼­\\StarCraft\\Maps\\Download\\qwer\\test_eud.scx");
        mr.extractChk();
        mr.parseChk();
        system("pause");
        return true;
    }
    catch (const std::runtime_error& e) {
        std::cout << e.what() << std::endl;
        system("pause");
    }

}

