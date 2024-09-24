#include "imgui_things.h"

void writeEUDVariable(std::reference_wrapper<EUDVariable>& obj, uint32_t param, uint32_t value) {
    Internal::dwwrite(obj.get( ).address, value);
}

void writeEUDArray(std::reference_wrapper<EUDVariable>& obj, uint32_t param, uint32_t value) {
    Internal::dwwrite(obj.get( ).address + param * 4, value);
}

void writeEUDVArray(std::reference_wrapper<EUDVariable>& obj, uint32_t param, uint32_t value) {
    Internal::dwwrite(getBaseAddr() + obj.get( ).address + 348 + param * 72, value);
}

void inputable_form(bool isHex, uint32_t param1, std::reference_wrapper<EUDVariable>& obj, int var_idx, void (*func)(std::reference_wrapper<EUDVariable>&, uint32_t, uint32_t)) {
    if (param1 >= obj.get( ).display_buf.size( ))
        return;
    
    if (isHex) {
        std::cout << "==================\n";
        if (obj.get( ).cgfw_type == "EUDArray" || obj.get( ).cgfw_type == "EUDVArray" || obj.get( ).cgfw_type == "PVariable") {
            snprintf(obj.get( ).display_buf[param1], 20, "0x%08X\0", obj.get( ).additional_value[param1]);
        }
        else {
            snprintf(obj.get( ).display_buf[param1], 20, "0x%08X\0", obj.get( ).value);
        }
        ImGui::SetNextItemWidth(90);
        ImGui::InputText(std::format("##form_hex{}", var_idx).c_str( ), obj.get( ).display_buf[param1], 20, ImGuiInputTextFlags_CharsHexadecimal);
        if (ImGui::IsItemDeactivatedAfterEdit( )) {
            (*func)(obj, param1, static_cast<uint32_t>(std::stoul(obj.get( ).display_buf[param1]+2, nullptr, 16)));
        }

    }
    else {
        if (obj.get( ).cgfw_type == "EUDArray" || obj.get( ).cgfw_type == "EUDVArray" || obj.get( ).cgfw_type == "PVariable") {
            snprintf(obj.get( ).display_buf[param1], 20, "%d\0", obj.get( ).additional_value[param1]);
        }
        else {
            snprintf(obj.get( ).display_buf[param1], 20, "%d\0", obj.get( ).value);
        }
        ImGui::SetNextItemWidth(90);
        ImGui::InputText(std::format("##form_dec{}", var_idx).c_str( ), obj.get( ).display_buf[param1], 20, ImGuiInputTextFlags_CharsDecimal);
        if (ImGui::IsItemDeactivatedAfterEdit( )) {
            (*func)(obj, param1, static_cast<uint32_t>(std::stoul(obj.get( ).display_buf[param1], nullptr, 10)));
        }
    }
}