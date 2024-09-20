#ifndef LOCATIONDRAW_H
#define LOCATIONDRAW_H
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include "drawing.h"
#include "variable.h"
#include "memoryRW.h"
#include "game_data.h"

class Location {
public:
    uint8_t index; // add 1 when displayed to user
    uint32_t top, left, down, right;
    std::string label;

    Location(uint8_t _index, const std::vector<uint8_t>& data);
    void update(const std::vector<uint8_t>& data);
};

class Locations {
public:
    uint32_t mrgn_addr;
    std::vector<Location> locations;
    std::vector<std::string> location_labels;
    uint32_t screen_center_x, screen_center_y;
    uint32_t screen_size_x, screen_size_y;
    
    uint32_t screen_top, screen_left;

    Locations(uint32_t _mrgn_addr, std::vector<std::string> _location_labels);
    void drawLocations(const std::unique_ptr<Variables>& var_ptr, GameData& game_data);
    void updateData( );
};


#endif