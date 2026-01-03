cmake_minimum_required(VERSION 3.20)
project(GFRMod)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MinHook
add_subdirectory(minhook)

# Injector (no console window)
add_executable(Injector WIN32 src/injector.cpp)
target_link_libraries(Injector PRIVATE user32)

# ImGui sources
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
)

# Mod source files (modular structure)
set(MOD_SOURCES
    src/dllmain.cpp
    src/il2cpp/il2cpp.cpp
    src/game/game.cpp
    src/features/settings.cpp
    src/features/aimbot.cpp
    src/features/norecoil.cpp
    src/features/fov.cpp
    src/features/esp.cpp
    src/features/weakness_hack.cpp
    src/hooks/hooks.cpp
    src/render/dx11.cpp
    src/render/menu.cpp
)

# Internal Mod DLL (inject into game)
add_library(InternalAimbot SHARED 
    ${MOD_SOURCES}
    ${IMGUI_SOURCES}
)
target_include_directories(InternalAimbot PRIVATE 
    minhook/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    src
)
target_link_libraries(InternalAimbot PRIVATE user32 minhook d3d11 dxgi)
