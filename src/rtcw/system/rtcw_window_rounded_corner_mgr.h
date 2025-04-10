/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (c) 2012-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef RTCW_WINDOW_ROUNDED_CORNER_MGR_INCLUDED
#define RTCW_WINDOW_ROUNDED_CORNER_MGR_INCLUDED

struct SDL_Window;

namespace rtcw {

class WindowRoundedCornerMgr
{
public:
	static void disable(SDL_Window* sdl_window);

private:
	class Impl;
};

} // namespace rtcw

#endif // RTCW_WINDOW_ROUNDED_CORNER_MGR_INCLUDED
