#include "drawing.h"

ImFont* Kostart3 = nullptr;
ImFont* Kostart5 = nullptr;
ImFont* Kostart8 = nullptr;
ImFont* Kostart10 = nullptr;
ImFont* Kostart15 = nullptr;
ImFont* Kostart20 = nullptr;
ImFont* Kostart25 = nullptr;
ImFont* Kostart30 = nullptr;

const char* ttf_path = "../bin/Kostar.ttf";
// since we cannot get the dir of dll, find other way, till then use ENG
void initFonts() {
    if (Kostart3) return;
    
    Kostart3 = ImGui::GetIO( ).Fonts->AddFontFromFileTTF(ttf_path, 3.0f);
    Kostart8 = ImGui::GetIO( ).Fonts->AddFontFromFileTTF(ttf_path, 8.0f);
    Kostart10 = ImGui::GetIO( ).Fonts->AddFontFromFileTTF(ttf_path, 10.0f);
    Kostart15 = ImGui::GetIO( ).Fonts->AddFontFromFileTTF(ttf_path, 15.0f);
    Kostart20 = ImGui::GetIO( ).Fonts->AddFontFromFileTTF(ttf_path, 20.0f);
    Kostart25 = ImGui::GetIO( ).Fonts->AddFontFromFileTTF(ttf_path, 25.0f);
    Kostart30 = ImGui::GetIO( ).Fonts->AddFontFromFileTTF(ttf_path, 30.0f);
    ImGui::GetIO( ).Fonts->Build( );
}

ImFont* getFont(int size) {
    ImFont* ret = nullptr;
    switch (size) {
        case 3:
            return Kostart3;
        case 5:
            return Kostart5;
        case 8:
            return Kostart8;
        case 10:
            return Kostart10;
        case 15:
            return Kostart15;
        case 20:
            return Kostart15;
        case 25:
            return Kostart15;
        case 30:
            return Kostart15;
        default:
            return nullptr;
    }
}

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
