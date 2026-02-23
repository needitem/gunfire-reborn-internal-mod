# Gunfire Reborn Mod

A DLL injection mod for Gunfire Reborn with various gameplay enhancements.

## Features

- **F1** - Silent Aim (auto-aim to weak points)
- **F2** - Infinite Ammo (no reload needed)
- **F3** - Speed Boost + Higher Jump
- **F4** - No Recoil
- **F5** - No Spread
- **F6** - Fast Bullet (100x speed)
- **F8** - FOV Hack (120 degree field of view)
- **F9** - Weakness Hit (all hits become weak point hits)
- **ALT** - ESP (secret rooms, treasure boxes)
- **Mouse Wheel** - Auto Pickup (teleport all items to you)
- **END** - Exit mod

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

## Usage

1. Build the project
2. Run `Injector.exe` while Gunfire Reborn is running
3. DLL will be injected automatically

## Version And Release

- Local version is managed in `VERSION` (example: `v1.3.3`).
- Injector auto-update target is fixed to `needitem/gunfire-reborn-internal-mod` GitHub Releases.
- Latest release DLL is downloaded when tag version is newer than local version.
- Release publishing is handled by `.github/workflows/release.yml` on `v*` tags.

## Requirements

- Windows 10/11
- Visual Studio 2019+ or CMake
- Gunfire Reborn (Steam version)

## Dependencies

- [MinHook](https://github.com/TsudaKageworker/minhook) - API hooking library
- [Dear ImGui](https://github.com/ocornut/imgui) - GUI library (optional)

## Disclaimer

For educational purposes only. Use at your own risk.
