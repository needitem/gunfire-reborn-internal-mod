# Gunfire Reborn Mod

A DLL injection mod for Gunfire Reborn with various gameplay enhancements.

## Features (Hotkeys)

- **HOME** - Toggle menu
- **F1** - Silent Aim (auto-aim to weak points)
- **F2** - Infinite Ammo (primary + perform ammo stay full)
- **F3** - Speed Boost + Higher Jump
- **F4** - No Recoil
- **F5** - No Spread
- **F6** - Fast Bullet (100x speed)
- **F10** - Weakness Hit (force weak-point style hit result)
- **F11** - No Cooldown
- **ALT (hold)** - ESP overlay (secret walls/portals, boxes, optional NPC)
- **Middle Mouse Button (Wheel Click)** - Auto Pickup
- **END** - Unload mod

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

## Usage

1. Build the project.
2. Place `Injector.exe` and `InternalAimbot.dll` in the same folder.
3. Run `Injector.exe` while Gunfire Reborn is running.
4. Injector checks latest GitHub release and updates `InternalAimbot.dll` automatically when a newer tag exists.
5. DLL is injected automatically after update check.

## Version and Release

- Local bundled version is managed in `VERSION` and compiled into Injector as `GFR_MOD_VERSION`.
- Injector stores last downloaded tag in `InternalAimbot.version` next to the executable.
- Auto-update target is fixed to `needitem/gunfire-reborn-internal-mod` latest release asset `InternalAimbot.dll`.
- Release publishing is handled by `.github/workflows/release.yml` on `v*` tags and uploads `build/Release/InternalAimbot.dll` and `build/Release/Injector.exe`.

## Requirements

- Windows 10/11
- Visual Studio 2019+ or CMake
- Gunfire Reborn (Steam version)

## Dependencies

- [MinHook](https://github.com/TsudaKageyu/minhook) - API hooking library
- [Dear ImGui](https://github.com/ocornut/imgui) - GUI library (optional)

## Disclaimer

For educational purposes only. Use at your own risk.
