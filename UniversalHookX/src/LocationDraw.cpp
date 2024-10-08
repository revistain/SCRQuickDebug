#include "LocationDraw.h"

Location::Location(uint8_t _index, const std::vector<uint8_t>& data) {
    index = _index;
    update(data);
}

void Location::update(const std::vector<uint8_t>& data) {
    if (data.size( ) < 16) {
        std::cout << "error while initializing location using vector\n";
    }
    std::memcpy(&left, data.data( ), sizeof(left));
    std::memcpy(&top, data.data( ) + 4, sizeof(top));
    std::memcpy(&right, data.data( ) + 8, sizeof(right));
    std::memcpy(&down, data.data( ) + 12, sizeof(down));
    left  = left  & 0x1FFF;
    top   = top   & 0x1FFF;
    right = right & 0x1FFF;
    down  = down  & 0x1FFF;
}

void Locations::updateData() {
    std::vector<uint8_t> data = Internal::vecread(mrgn_addr, 20 * 255);
    for (size_t i = 0; i < locations.size(); i++) {
        const std::vector<uint8_t> subdata(data.begin( ) + i * 20, data.begin( ) + i * 20 + 20);
        locations[i].update(subdata);
    }
}

///////////////////////////////////
Locations::Locations(uint32_t _mrgn_addr, std::vector<std::string> _location_labels) :
    mrgn_addr(_mrgn_addr), screen_center_x(0), screen_center_y(0), screen_top(0), screen_left(0), location_labels(_location_labels) {
    std::vector<uint8_t> data = Internal::vecread(mrgn_addr, 20 * 255);
    for (size_t i = 0; i < 255; i++) {
        const std::vector<uint8_t> subdata(data.begin( ) + i * 20, data.begin( ) + i * 20 + 20);
        locations.emplace_back((uint8_t)(i), subdata);
        locations[i].label = location_labels[i];
    }
}

void Locations::drawLocations(const std::unique_ptr<Variables>& var_ptr, GameData& game_data, bool* isLocationVisible) {
    screen_left = var_ptr->screenTL[1];
    screen_top  = var_ptr->screenTL[0];
    screen_size_x = game_data.screen_size_x;
    screen_size_y = game_data.screen_size_y;
    screen_center_x = game_data.screen_size_x / 2 + screen_left;
    screen_center_y = game_data.screen_size_y / 2 + screen_top;

    for (size_t idx = 0; idx < 255; idx++) {
        if (!var_ptr->LocationsUse[idx] || !isLocationVisible[idx]) {
            continue;
        }
        if (locations[idx].right >= screen_left ||
            locations[idx].left < screen_left + screen_size_x ||
            locations[idx].down >= screen_top ||
            locations[idx].top < screen_top + screen_size_y) {
            int draw_x = locations[idx].left - screen_left;
            int draw_y = locations[idx].top - screen_top;
            uint32_t draw_size_x = abs(static_cast<int>(locations[idx].left - locations[idx].right));
            uint32_t draw_size_y = abs(static_cast<int>(locations[idx].top - locations[idx].down));
            DrawSquareWithLabel(game_data.pillar_size, draw_x, draw_y, draw_size_x, draw_size_y, locations[idx].label.c_str( ), locations[idx].index);
        }
    }
}