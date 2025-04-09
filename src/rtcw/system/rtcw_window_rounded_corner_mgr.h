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
