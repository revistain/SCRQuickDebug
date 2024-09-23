#include "drawing.h"

void DrawSquare(float x, float y, float sizeX, float sizeY, int seed = 0) {
    ImDrawList* drawList = ImGui::GetWindowDrawList( );

    float ratio = getRatio( );
    if (!ratio)
        return;
    sizeX = ratio * sizeX;
    sizeY = ratio * sizeY;

    ImVec2 topLeft(x, y);
    ImVec2 bottomRight(x + sizeX, y + sizeY);

    srand(seed);
    ImU32 color = IM_COL32(rand( ) % 75 + 180, rand( ) % 75 + 180, rand( ) % 75 + 180, rand( ) % 20 + 10); // RGBA

    drawList->AddRectFilled(topLeft, bottomRight, color);
}

void DrawSquareWithLabel(uint32_t pillar_size, float x, float y, float sizeX, float sizeY, const char* label, int seed = 0) {
    ImDrawList* drawList = ImGui::GetWindowDrawList( );

    float ratio = getRatio( );
    if (!ratio) return;
    x = ratio * x + pillar_size;
    y = ratio * y;
    sizeX = ratio * sizeX;
    sizeY = ratio * sizeY;
    ImU32 dotColor = IM_COL32(rand( ) % 75 + 180, rand( ) % 75 + 180, rand( ) % 75 + 180, 250);
    ImU32 squareColor = IM_COL32(rand( ) % 75 + 180, rand( ) % 75 + 180, rand( ) % 75 + 180, rand( ) % 20 + 30); // RGBA
    ImU32 colorToUse = squareColor;
    /*
    // help visualize dot location
    if (sizeX < 3 && sizeY < 3)
        colorToUse = dotColor;
        if (sizeX < 3) sizeX = 3;
        else if (sizeY < 3) sizeY = 3;
    else 
        colorToUse = squareColor;
    */

    ImVec2 topLeft(x, y);
    ImVec2 bottomRight(x + sizeX, y + sizeY);

    srand(seed);
    drawList->AddRectFilled(topLeft, bottomRight, colorToUse);

    ImVec2 labelSize = ImGui::CalcTextSize(label);
    ImVec2 labelPos(x + 6, y + 4);

    ImU32 textColor = IM_COL32(255, 255, 255, 255); // RGBA
    drawList->AddText(labelPos, textColor, label);
}
