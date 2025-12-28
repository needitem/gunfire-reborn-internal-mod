cmake_minimum_required(VERSION 3.20)
project(GFRMod)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MinHook
add_subdirectory(minhook)

# Injector (no console window)
add_executable(Injector WIN32 src/injector.cpp)
target_link_libraries(Injector PRIVATE user32)

# Internal Mod DLL (inject into game)
add_library(InternalAimbot SHARED src/internal_aimbot.cpp)
target_include_directories(InternalAimbot PRIVATE minhook/include)
target_link_libraries(InternalAimbot PRIVATE user32 minhook)
