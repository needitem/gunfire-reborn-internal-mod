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

# Internal Mod DLL (inject into game)
add_library(InternalAimbot SHARED 
    src/internal_aimbot.cpp
    ${IMGUI_SOURCES}
)
target_include_directories(InternalAimbot PRIVATE 
    minhook/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(InternalAimbot PRIVATE user32 minhook d3d11 dxgi)
