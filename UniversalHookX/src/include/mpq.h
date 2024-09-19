//#ifndef MPQ_H
//#define MPQ_H
//#include "section.h"
//#include <iostream>
//#include <string>
//#include <vector>
//#include <stdexcept>
//#include <algorithm>
//#include <cstring>
//#include <windows.h>
//#include <SFmpq_static.h>
//
//class MPQRead {
//private:
//    uint32_t fsize;
//    HANDLE hMPQ;
//    HANDLE hFILE;
//    std::vector<uint8_t> chk;
//    std::vector<std::string> sectionNames = {
//        "VER ", "VCOD", "OWNR", "SIDE", "COLR", "ERA ", "DIM ", "MTXM",
//        "UNIT", "PUNI", "UNIx", "PUPx", "UPGx", "THG2", "MASK", "MRGN",
//        "SPRP", "FORC", "PTEx", "TECx", "MBRF", "TRIG", "UPRP", "STRx",
//        "STR "// , "ISOM", "STUB"
//        // freeze makes ISOM length and data strange, so just ignore :) also ignore freeze made STUB
//    };
//
//    // TODO: make in to name section pair; or just make find function there are only few so...
//    std::vector<Section> sections;
//public:
//    MPQRead() : fsize(0), hMPQ(NULL), hFILE(NULL) {}
//    ~MPQRead() {}
//    void open(const char* map_name);
//    void extractChk();
//    void parseChk();
//    Section& findSectionByName(const char* name);
//    std::vector<uint8_t>& getChk() { return chk; }
//
//};
//bool extractMPQ();
//#endif
