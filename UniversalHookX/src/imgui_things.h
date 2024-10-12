#ifndef IMGUI_THINGS_H
#define IMGUI_THINGS_H
#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/imgui_impl_win32.h"
#include "../dependencies/imgui/imgui_internal.h"

#include "variable.h"
#include "signature.h"
#include "memoryRW.h"
#include <functional>
#include <format>


void writeEUDVariable(EUDVariable& obj, uint32_t param, uint32_t value);
void writeEUDArray(EUDVariable& obj, uint32_t param, uint32_t value);
void writeEUDVArray(EUDVariable& obj, uint32_t param, uint32_t value);

void inputable_form(bool isHex, uint32_t param1, EUDVariable& obj, int var_idx, void (*func)(EUDVariable&, uint32_t, uint32_t));
inline void ToggleButton(const char* str_id, bool* v) {
    ImVec4* colors = ImGui::GetStyle( ).Colors;
    ImVec2 p = ImGui::GetCursorScreenPos( );
    ImDrawList* draw_list = ImGui::GetWindowDrawList( );

    float height = ImGui::GetFrameHeight( );
    float width = height * 1.55f;
    float radius = height * 0.50f;
    float rounding = 0.2f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked( ))
        *v = !*v;
    ImGuiContext& gg = *GImGui;
    float ANIM_SPEED = 0.025f;
    if (gg.LastActiveId == gg.CurrentWindow->GetID(str_id)) // && g.LastActiveIdTimer < ANIM_SPEED)
        float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
    if (ImGui::IsItemHovered( ))
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * rounding);
    else
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * rounding);

    ImVec2 center = ImVec2(radius + (*v ? 1 : 0) * (width - radius * 2.0f), radius);
    draw_list->AddRectFilled(ImVec2((p.x + center.x) - 9.0f, p.y + 1.5f),
                             ImVec2((p.x + (width / 2) + center.x) - 9.0f, p.y + height - 1.5f), IM_COL32(255, 255, 255, 255), height * rounding);
}

namespace ImGui {
    void CenterAlign(std::string text);
}
#endif