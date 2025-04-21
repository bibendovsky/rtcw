# RTCW: Unofficial source port for Return to Castle Wolfenstein and Enemy Territory
# SDL2 changelog

### Removed
- Directory *test*.

### Changed
- *CMakeLists.txt*: Minimum version to v3.5.0.
- *CMakeLists.txt*: Enforce C++ to build macOS release.
- *src/dynapi/SDL_dynapi.h*: Disable dynamic API.
- *CMakeLists.txt*: Configure static linking.
- *CMakeLists.txt*: Fix using DXSDK_DIR in older CMake.
- *CMakeLists.txt*: Don use use multi-process compilation for Visual C++ 7.1 (2003) or lower.
