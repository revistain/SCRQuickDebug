#include "imgui_things.h"

void writeEUDVariable(EUDVariable& obj, uint32_t param, uint32_t value) {
    Internal::dwwrite(obj.address, value & 0xFFFFFFFF);
}

void writeEUDArray(EUDVariable& obj, uint32_t param, uint32_t value) {
    Internal::dwwrite(obj.address + param * 4, value & 0xFFFFFFFF);
}

void writeEUDVArray(EUDVariable& obj, uint32_t param, uint32_t value) {
    Internal::dwwrite(getBaseAddr() + obj.address + 348 + param * 72, value & 0xFFFFFFFF);
}

void inputable_form(bool isHex, uint32_t param1, EUDVariable& obj, int var_idx, void (*func)(EUDVariable&, uint32_t, uint32_t)) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // Background color
    if (isHex) {
        if (obj.cgfw_type == "EUDArray" || obj.cgfw_type == "EUDVArray" || obj.cgfw_type == "PVariable") {
            obj.display_buf[param1] = std::format("{:08X}\0", obj.additional_value[param1]);
        } else {
            obj.display_buf[param1] = std::format("{:08X}\0", obj.value);
        }
        char dummy[3] = "0x";
        ImGui::SetNextItemWidth(20);
        ImGui::InputText(std::format("##hex_prefix{}", var_idx).c_str( ), dummy, 2, ImGuiInputTextFlags_ReadOnly);
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::SetNextItemWidth(70);
        ImGui::InputText(std::format("##form_hex{}", var_idx).c_str( ), &obj.display_buf[param1][0], 9, ImGuiInputTextFlags_CharsHexadecimal);
        if (ImGui::IsItemDeactivatedAfterEdit( )) {
            (*func)(obj, param1, static_cast<uint32_t>(std::stoul(obj.display_buf[param1], nullptr, 16)));
        }

    } else {
        if (obj.cgfw_type == "EUDArray" || obj.cgfw_type == "EUDVArray" || obj.cgfw_type == "PVariable") {
            obj.display_buf[param1] = std::format("{}\0", obj.additional_value[param1]);
        } else {
            obj.display_buf[param1] = std::format("{}\0", obj.value);
        }
        ImGui::SetNextItemWidth(100);
        ImGui::InputText(std::format("##form_dec{}", var_idx).c_str( ), &obj.display_buf[param1][0], 15, ImGuiInputTextFlags_CharsDecimal);
        if (ImGui::IsItemDeactivatedAfterEdit( )) {
            (*func)(obj, param1, static_cast<uint32_t>(std::stoul(obj.display_buf[param1], nullptr, 10)));
        }
    }
    ImGui::PopStyleColor(1);
}