#include "imgui_things.h"

void writeEUDVariable(std::reference_wrapper<EUDVariable>& obj, uint32_t offset, uint32_t value) {
    Internal::dwwrite(obj.get( ).address + offset, value);
}

void inputable_form(bool isHex, std::reference_wrapper<EUDVariable>& obj, int var_idx, void (*func)(std::reference_wrapper<EUDVariable>&, uint32_t, uint32_t)) {
    obj.get( ).display_buf[0] = obj.get( ).value;
    if (isHex) {
        char buf[11];
        snprintf(buf, sizeof(buf), "0x%08X", obj.get( ).display_buf[0]); // 16진수로 변환
        ImGui::SetNextItemWidth(90);
        ImGui::InputText(std::format("##var_hex{}", var_idx).c_str( ), buf, sizeof(buf), ImGuiInputTextFlags_CharsHexadecimal);
        if (ImGui::IsItemDeactivatedAfterEdit( )) {
            std::cout << "foucus\n";
            std::cout << "buf: " << std::hex << buf << " / " << static_cast<uint32_t>(std::stoul(buf + 2, nullptr, 16)) << "\n";
            (*func)(obj, 0, static_cast<uint32_t>(std::stoul(buf + 2, nullptr, 16)));
            
        }
    } else {
        char buf[20];
        snprintf(buf, sizeof(buf), "%d", obj.get( ).display_buf[0]); // 10진수로 변환
        ImGui::SetNextItemWidth(90);
        ImGui::InputText(std::format("##var_dec{}", var_idx).c_str( ), buf, sizeof(buf), ImGuiInputTextFlags_CharsDecimal);
        if (ImGui::IsItemDeactivatedAfterEdit( )) {
            std::cout << "foucus\n";
            Internal::dwwrite(obj.get( ).address, static_cast<uint32_t>(std::stoul(buf, nullptr, 10)));
        }
    }
}