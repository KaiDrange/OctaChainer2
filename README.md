# OctaChainer2

JUCE audio project configured with CMake for CLion.

## Prerequisites

- CMake 3.22+
- A C++17 compiler
  - Windows: Visual Studio 2022 Build Tools (MSVC)
  - macOS: Xcode + Command Line Tools
- Git (for JUCE FetchContent)

## Configure and build

This repo includes `CMakePresets.json`, so CLion can import ready-made build profiles.

### Windows (PowerShell)

```powershell
cmake -S . -B build -DOCTA_BUILD_VST3=OFF
cmake --build build --config Debug
```

### macOS (zsh/bash)

```bash
cmake -S . -B build -DOCTA_BUILD_VST3=OFF
cmake --build build --config Debug
```

To enable VST3 as well as standalone:

```powershell
cmake -S . -B build -DOCTA_BUILD_VST3=ON
cmake --build build --config Debug
```

## License

This project is licensed under the GNU General Public License v3.0.
See `LICENSE` for the full text.

