#include "font.h"
#include "dllmain.h"

HRSRC hResource;
HGLOBAL hLoadedResource;
void* pResourceData;

void LoadFontResource( ) {
    HINSTANCE hInstance = getDllHandle();
    std::cout << "hinstance: " << hInstance << "\n";
    HMODULE hMod = LoadLibraryW(L"eudplib_debug.dll");
    hResource = FindResource(hInstance, L"IDR_FONT1", RT_RCDATA);

    if (hResource == NULL) {
        DWORD error = GetLastError( );
        std::wcout << L"Cannot find font resource. Error code: " << error << std::endl;
        return;
    }

    hLoadedResource = LoadResource(hInstance, hResource);
    if (hLoadedResource == NULL) {
        DWORD error = GetLastError( );
        std::wcout << L"Failed to load resource. Error code: " << error << std::endl;
        return;
    }

    pResourceData = LockResource(hLoadedResource);
    DWORD resourceSize = SizeofResource(hInstance, hResource);

    if (resourceSize == 0 || pResourceData == NULL) {
        std::wcout << L"Invalid font resource data or size is zero." << std::endl;
        return;
    }

    std::wcout << L"Font data size: " << resourceSize << L" bytes" << std::endl;

    if (AddFontMemResourceEx(pResourceData, resourceSize, NULL, NULL) == 0) {
        std::wcout << L"Failed to add font from resource." << std::endl;
    }
}

ImFont* Kostart3 = nullptr;
ImFont* Kostart5 = nullptr;
ImFont* Kostart8 = nullptr;
ImFont* Kostart10 = nullptr;
ImFont* Kostart15 = nullptr;
ImFont* Kostart20 = nullptr;
ImFont* Kostart25 = nullptr;
ImFont* Kostart30 = nullptr;

void initFonts( ) {
    return;
    if (Kostart3)
        return;

    LoadFontResource( );

    // Create ImGui context if not already created
    ImGuiIO& io = ImGui::GetIO( );

    // Check resource size
    DWORD resourceSize = SizeofResource(GetModuleHandle(NULL), hResource);
    if (resourceSize == 0 || pResourceData == nullptr) {
        std::cout << "Invalid font resource data!" << std::endl;
        return;
    }

    // Load fonts of different sizes
    Kostart3 = io.Fonts->AddFontFromMemoryTTF(pResourceData, resourceSize, 3.0f, nullptr, io.Fonts->GetGlyphRangesDefault( ));
    Kostart8 = io.Fonts->AddFontFromMemoryTTF(pResourceData, resourceSize, 8.0f, nullptr, io.Fonts->GetGlyphRangesDefault( ));
    Kostart10 = io.Fonts->AddFontFromMemoryTTF(pResourceData, resourceSize, 10.0f, nullptr, io.Fonts->GetGlyphRangesDefault( ));
    Kostart15 = io.Fonts->AddFontFromMemoryTTF(pResourceData, resourceSize, 15.0f, nullptr, io.Fonts->GetGlyphRangesDefault( ));
    Kostart20 = io.Fonts->AddFontFromMemoryTTF(pResourceData, resourceSize, 20.0f, nullptr, io.Fonts->GetGlyphRangesDefault( ));
    Kostart25 = io.Fonts->AddFontFromMemoryTTF(pResourceData, resourceSize, 25.0f, nullptr, io.Fonts->GetGlyphRangesDefault( ));
    Kostart30 = io.Fonts->AddFontFromMemoryTTF(pResourceData, resourceSize, 30.0f, nullptr, io.Fonts->GetGlyphRangesDefault( ));
    std::cout << "font:" << std::hex << Kostart10 << "\n";
    // Check if fonts were loaded successfully
    if (!Kostart3 || !Kostart8 || !Kostart10 || !Kostart15 || !Kostart20 || !Kostart25 || !Kostart30) {
        std::cerr << "Failed to load one or more fonts!" << std::endl;
        return; // Handle error accordingly
    }

    // Build the font atlas
    io.Fonts->Build( );
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
