#ifndef LOCATIONDRAW_H
#define LOCATIONDRAW_H
#include <cstdint>
#include <vector>
#include <string>
#include "drawing.h"

typedef struct {
    uint32_t mapX, mapY, sizeX, sizeY;
}Location;

class Locations {
public:
    std::vector<Location> locations;
    std::string label;
    uint32_t screen_center_x, screen_center_y;
    uint32_t screen_top, screen_left;

    Locations() {

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
                DrawSquareWithLabel(draw_x, draw_y, draw_size_x, draw_size_y, label.c_str());
            }
        }
    }
};

#endif
