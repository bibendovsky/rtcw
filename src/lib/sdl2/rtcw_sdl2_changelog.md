# RTCW: Unofficial source port for Return to Castle Wolfenstein and Enemy Territory
# SDL2 changelog

### Removed
- Directory *test*.

### Changed
- *CMakeLists.txt*: Minimum version to v3.5.0.
- *src/dynapi/SDL_dynapi.h*: Disable dynamic API.
- *CMakeLists.txt*: Configure static linking.
- *CMakeLists.txt*: Fix using DXSDK_DIR in older CMake.
- *CMakeLists.txt*: Do not use use multi-process compilation for Visual C++ 7.1 (2003) or lower.
- *src/joystick/hidapi/SDL_hidapi_steam.c*: Disable DPRINTF.
- *src/filesystem/windows/SDL_sysfilesystem.c*: Include "shellapi.h" before the "shlobj.h".
- *src/joystick/windows/SDL_windowsjoystick.c*: WINDOWS_JoystickGetDeviceSteamVirtualGamepadSlot: Return virtual slot if SDL_JOYSTICK_XINPUT not defined.
- *src/thread/windows/SDL_syscond_cv.c*: Do not define CONDITION_VARIABLE for OpenWatcom.
- *src/thread/windows/SDL_sysmutex_c.h*: Do not define SRWLOCK for OpenWatcom.
- *CMakeLists.txt*: Disable resource files for Windows.
- *CMakeLists.txt*: Link to dxguid.lib when using non-WindowsSDK DirectX.
