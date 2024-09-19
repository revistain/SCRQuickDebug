#ifndef LOCATIONDRAW_H
#define LOCATIONDRAW_H
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include "drawing.h"
#include "variable.h"

typedef struct {
    uint32_t mapX, mapY, sizeX, sizeY;
    std::string label;
} Location;

class Locations {
public:
    uint32_t mrgn_addr;
    std::vector<Location> locations;
    uint32_t screen_center_x, screen_center_y;
    uint32_t screen_top, screen_left;

    Locations(uint32_t _mrgn_addr):
        mrgn_addr(_mrgn_addr),
        screen_center_x(0),
        screen_center_y(0),
        screen_top(0),
        screen_left(0) {
 
    }

    void drawLocations() {
        for (auto& loc : locations) {
            if (loc.mapX + loc.sizeX >= screen_left &&
                loc.mapX < screen_left + screen_center_x + screen_center_x &&
                loc.mapY + loc.sizeY >= screen_top &&
                loc.mapY < screen_top + screen_center_y + screen_center_y) {
                uint32_t draw_x = screen_center_x + screen_left - loc.mapX;
                uint32_t draw_y = screen_center_y + screen_top - loc.mapY;
                uint32_t draw_size_x = abs(static_cast<int>(screen_left - (loc.mapX + loc.sizeX)));
                uint32_t draw_size_y = abs(static_cast<int>(screen_top  - (loc.mapY + loc.sizeY)));
                DrawSquareWithLabel(draw_x, draw_y, draw_size_x, draw_size_y, loc.label.c_str());
            }
        }
    }
    void findMRGNData(std::unique_ptr<Variables>& var_ptr) {
        std::string map_path = var_ptr->mapPath;


    }

    void updateData(std::unique_ptr<Variables>& var_ptr) {

    }
};


#endif
