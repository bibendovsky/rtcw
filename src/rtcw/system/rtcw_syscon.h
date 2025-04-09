#ifndef RTCW_SYSCON_INCLUDED
#define RTCW_SYSCON_INCLUDED

union SDL_Event;

namespace rtcw {

class Syscon
{
public:
	enum ShowMode
	{
		show_mode_normal,
		show_mode_hidden,
		show_mode_minimized
	};

	enum CallbackType
	{
		callback_type_input_line,
		callback_type_show_mode,
		callback_type_quit
	};

	struct CallbackQuitTag {};

	struct CallbackParam
	{
		CallbackType type;

		union
		{
			const char* input_line_text;
			ShowMode show_mode;
		};

		explicit CallbackParam(const char* input_line_text)
			:
			type(callback_type_input_line),
			input_line_text(input_line_text)
		{}

		explicit CallbackParam(ShowMode show_mode)
			:
			type(callback_type_show_mode),
			show_mode(show_mode)
		{}

		explicit CallbackParam(CallbackQuitTag)
			:
			type(callback_type_quit),
			input_line_text()
		{}
	};

	typedef void (* Callback)(CallbackParam param);

public:
	Syscon();
	~Syscon();

	bool is_initialized() const;
	const char* get_error_message() const;
	const char* get_text() const;
	bool initialize(Callback callback);
	void terminate();
	void set_title(const char* title);
	void show(ShowMode mode);
	void clear_text();
	void append_text(const char* text);
	void set_error_text(const char* text);
	void update();
	void handle_event(const SDL_Event& sdl_event);
	void run();

private:
	class Impl;

private:
	Impl* impl_;

private:
	Syscon(const Syscon&);
	Syscon& operator=(const Syscon&);
};

} // namespace rtcw

#endif // RTCW_SYSCON_INCLUDED
