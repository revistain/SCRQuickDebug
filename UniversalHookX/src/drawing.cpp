#include "drawing.h"

void DrawSquare(float x, float y, float sizeX, float sizeY) {
    ImDrawList* drawList = ImGui::GetWindowDrawList( );

    float ratio = getRatio( );
    if (!ratio)
        return;
    int seed = static_cast<int>(x + y + sizeX + sizeY);
    sizeX = ratio * sizeX;
    sizeY = ratio * sizeY;

    // Define the corners of the square
    ImVec2 topLeft(x, y);
    ImVec2 bottomRight(x + sizeX, y + sizeY);

    // Set color (for example, red)
    srand(seed);
    ImU32 color = IM_COL32(rand( ) % 75 + 180, rand( ) % 75 + 180, rand( ) % 75 + 180, rand( ) % 20 + 10); // RGBA

    // Draw the filled square
    drawList->AddRectFilled(topLeft, bottomRight, color);
}

void DrawSquareWithLabel(float x, float y, float sizeX, float sizeY, const char* label) {
    ImDrawList* drawList = ImGui::GetWindowDrawList( );

    float ratio = getRatio( );
    if (!ratio)
        return;
    int seed = static_cast<int>(x + y + sizeX + sizeY);
    sizeX = ratio * sizeX;
    sizeY = ratio * sizeY;

    // Define the corners of the square
    ImVec2 topLeft(x, y);
    ImVec2 bottomRight(x + sizeX, y + sizeY);

    // Set color for the square (e.g., red)
    srand(seed);
    ImU32 squareColor = IM_COL32(rand( ) % 75 + 180, rand( ) % 75 + 180, rand( ) % 75 + 180, rand( ) % 20 + 30); // RGBA
    drawList->AddRectFilled(topLeft, bottomRight, squareColor);

    // Calculate label position (centered)
    ImVec2 labelSize = ImGui::CalcTextSize(label);
    ImVec2 labelPos(x + 20, y + 10);

    // Set color for the text (e.g., white)
    ImU32 textColor = IM_COL32(255, 255, 255, 255); // RGBA
    drawList->AddText(labelPos, textColor, label);
}
