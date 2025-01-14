#include "rtcw_syscon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <new>
#include "SDL.h"
#include "rtcw_sdl_utility.h"
#include "rtcw_syscon_font_16x8.h"
#include "rtcw_array_trivial.h"
#include "rtcw_vector_trivial.h"

// ==========================================================================

namespace rtcw
{

class Syscon::Impl
{
public:
	Impl();
	~Impl();

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
	enum Keyindex
	{
		keyindex_none = -1,

		keyindex_lctrl,
		keyindex_rctrl,
		keyindex_lshift,
		keyindex_rshift,

		keyindex_tab,
		keyindex_backspace,
		keyindex_space,
		keyindex_enter,
		keyindex_insert,
		keyindex_delete,
		keyindex_home,
		keyindex_end,
		keyindex_page_up,
		keyindex_page_down,
		keyindex_left,
		keyindex_right,
		keyindex_up,
		keyindex_down,

		keyindex_count_
	};

	enum ScrollbarArrowDirection
	{
		scrollbar_arrow_direction_down,
		scrollbar_arrow_direction_up
	};

private:
	static const unsigned int min_redraw_duartion_ms = 20;

	static const int error_message_max_length = 2047;
	static const int error_message_max_size = error_message_max_length + 1;

	static const int ui_input_line_max_length = 128;
	static const int ui_input_line_max_size = ui_input_line_max_length + 1;

	static const int unicode_replacement_code_point = 0xFFFD;

	static const int unicode_min_surrogate = 0xD800;
	static const int unicode_max_surrogate = 0xDFFF;

	static const int utf8_first_code_point_1 = 0x000000;
	static const int utf8_last_code_point_1 = 0x00007F;

	static const int utf8_first_code_point_2 = 0x000080;
	static const int utf8_last_code_point_2 = 0x0007FF;

	static const int utf8_first_code_point_3 = 0x000800;
	static const int utf8_last_code_point_3 = 0x00FFFF;

	static const int utf8_first_code_point_4 = 0x010000;
	static const int utf8_last_code_point_4 = 0x10FFFF;

	static const int window_min_width = 640;
	static const int window_min_height = 480;
	static const int font_max_glyphs = 256;
	static const int font_first_code_point = ' ';
	static const int font_missing_code_point = '?';
	static const int font_glyph_width = 8;
	static const int font_glyph_height = 16;
	static const int font_glyph_count = 190;
	static const int font_atlas_width = 256;
	static const int font_atlas_height = 128;
	static const int font_atlas_area = font_atlas_width * font_atlas_height;
	static const int font_atlas_glyphs_per_width = font_atlas_width / font_glyph_width;
	static const int font_atlas_glyphs_per_height = font_atlas_height / font_glyph_height;

	static const int max_focusable_controls = 5;

	static const int default_border_width = 1;
	static const int default_extra_border_width = 2;

	static const int tab_size = 8;

	static const SDL_Color color_window;

	static const SDL_Color color_button_text;
	static const SDL_Color color_button_border;
	static const SDL_Color color_button_background;
	static const SDL_Color color_button_focused;
	static const SDL_Color color_button_highlighted;
	static const SDL_Color color_button_pressed;

	static const SDL_Color color_scrollbar_track;
	static const SDL_Color color_scrollbar_thumb;
	static const SDL_Color color_scrollbar_thumb_highlighted;
	static const SDL_Color color_scrollbar_thumb_pressed;
	static const SDL_Color color_scrollbar_button;
	static const SDL_Color color_scrollbar_button_highlighted;
	static const SDL_Color color_scrollbar_button_pressed;
	static const SDL_Color color_scrollbar_arrow;
	static const SDL_Color color_scrollbar_arrow_highlighted;
	static const SDL_Color color_scrollbar_arrow_pressed;

	static const SDL_Color color_text_box_border;
	static const SDL_Color color_text_box_border_highlighted;
	static const SDL_Color color_text_box_border_focused;
	static const SDL_Color color_text_box_background;
	static const SDL_Color color_text_box_text;

	static const SDL_Color color_edit_box_border;
	static const SDL_Color color_edit_box_border_highlighted;
	static const SDL_Color color_edit_box_border_focused;
	static const SDL_Color color_edit_box_background;
	static const SDL_Color color_edit_box_text;

	static const SDL_Color color_ui_error_label_text;

	static const int default_margin = 6;

	static const int default_label_padding = 2;

	static const int default_button_padding = 4;
	static const int default_button_width_in_glyphs = 11;

	static const int error_label_max_text_length = 1024;
	static const int error_label_line_count = 4;

	static const int default_edit_box_padding = 3;

	static const int scrollbar_min_thumb_height = 4;
	static const int scrollbar_arrow_width = 11;
	static const int scrollbar_arrow_height = 9;
	static const int scrollbar_button_height = 23;
	static const int default_scrollbar_width = 25;

	static const int ui_log_text_capacity = 32767;
	static const int text_box_text_max_length = ui_log_text_capacity - 1;
	static const int default_text_box_padding = 2;
	static const int edit_box_cursor_peek_length = 9;
	static const int edit_box_cursor_blinking_ticks = 500;

	static const int mouse_button_repeating_period_ticks = 30;
	static const int mouse_wheel_lines_per_scroll = 3;

private:
	struct FontGlyph
	{
		unsigned char atlas_x; // Left.
		unsigned char atlas_y; // Top.
	};

	typedef VectorTrivial<unsigned char> VectorU8;
	typedef VectorTrivial<unsigned short> VectorU16;
	typedef VectorTrivial<char> Text;
	typedef VectorTrivial<unsigned short> LineOffsets;

	struct Label
	{
		SDL_Rect rect;
		VectorU8 glyph_indices;
	};

	typedef void (Impl::* ButtonClickedCallback)();
	typedef void (Impl::* EditBoxTextCallback)(const char* text);

	struct Button
	{
		bool is_highlighted;
		bool is_pressed;
		SDL_Rect rect;
		VectorU8 glyph_indices;
		ButtonClickedCallback clicked_callback;

		Button()
			:
			is_highlighted(),
			is_pressed(),
			rect(),
			glyph_indices(),
			clicked_callback()
		{}
	};

	struct Scrollbar
	{
		bool is_visible;
		bool has_thumb;
		bool has_buttons;
		bool is_thumb_highlighted;
		bool is_thumb_pressed;
		bool is_upper_button_highlighted;
		bool is_upper_button_pressed;
		bool is_lower_button_highlighted;
		bool is_lower_button_pressed;
		SDL_Rect rect;
		SDL_Rect thumb_rect;
		SDL_Rect upper_button_rect;
		SDL_Rect lower_button_rect;
		int value;
		int min_value;
		int max_value;
		int line_size;
		int page_size;
	};

	struct TextBox
	{
		bool is_highlighted;

		SDL_Rect rect;
		SDL_Rect background_rect;
		SDL_Rect text_rect;
		Scrollbar scrollbar;

		SDL_Color border_color;
		SDL_Color highlighted_border_color;
		SDL_Color focused_border_color;
		SDL_Color background_color;
		SDL_Color text_color;

		Text text;
		VectorU8 glyph_indices;
		LineOffsets line_offsets;

		TextBox()
			:
			is_highlighted(),
			rect(),
			background_rect(),
			text_rect(),
			scrollbar(),
			border_color(),
			highlighted_border_color(),
			focused_border_color(),
			background_color(),
			text_color(),
			text(),
			glyph_indices(),
			line_offsets()
		{}
	};

	struct EditBox
	{
		bool is_highlighted;

		SDL_Rect rect;
		SDL_Rect text_rect;

		Text text;
		VectorU8 glyph_indices;

		int cursor_glyph_index;
		int window_pixel_offset;

		EditBoxTextCallback text_callback;

		EditBox()
			:
			is_highlighted(),
			rect(),
			text_rect(),
			glyph_indices(),
			cursor_glyph_index(),
			window_pixel_offset(),
			text_callback()
		{}
	};

	typedef ArrayTrivial<bool, keyindex_count_> PressedKeys;
	typedef ArrayTrivial<bool, keyindex_count_> RepeatedKeys;

	struct InputState
	{
		bool is_primary_button_pressed;
		bool is_primary_button_repeated;
		Uint32 primary_button_last_repeat_ticks;
		int mouse_wheel_v_delta;
		PressedKeys pressed_keys;
		RepeatedKeys repeated_keys;
		VectorU8 glyph_indices;

		InputState()
		{
			reset();
		}

		void reset()
		{
			is_primary_button_pressed = false;
			is_primary_button_repeated = false;
			primary_button_last_repeat_ticks = 0;
			mouse_wheel_v_delta = 0;
			pressed_keys.fill(false);
			repeated_keys.fill(false);
			glyph_indices.clear();
		}
	};

	typedef ArrayTrivial<char, error_message_max_size> ErrorMessage;
	typedef ArrayTrivial<void*, max_focusable_controls> FocusableControls;
	typedef ArrayTrivial<FontGlyph, font_max_glyphs> FontGlyphs;

private:
	bool is_initialized_;
	bool is_sdl_at_least_2_0_4;
	bool is_error_mode_;
	bool has_mouse_focus_;
	bool has_keyboard_focus_;
	Callback callback_;
	SdlSubsystem sdl_subsystem_;
	SdlWindowUPtr sdl_window_uptr_;
	SdlRendererUPtr sdl_renderer_uptr_;
	Uint32 sdl_window_id_;
	double scale_factor_;
	int window_width_;
	int window_height_;
	int font_glyph_width_;
	int font_glyph_height_;
	int margin_;
	int button_padding_;
	int button_height_;
	int button_width_;
	SdlTextureUPtr font_atlas_;
	FontGlyphs font_glyphs_;
	SDL_Point mouse_position_;
	TextBox ui_error_label_;
	Button ui_copy_button_;
	Button ui_clear_button_;
	Button ui_quit_button_;
	TextBox ui_log_;
	EditBox ui_input_line_;
	void* focused_control_;
	int focusable_controls_count_;
	Scrollbar* dragging_scrollbar_;
	int dragging_scrollbar_anchor_;
	bool edit_box_cursor_is_visible_;
	Uint32 edit_box_cursor_last_ticks_;
	FocusableControls focusable_controls_;
	InputState old_input_state_;
	InputState input_state_;
	ErrorMessage error_message_;

private:
	static const unsigned short scrollbar_up_arrow_bitmap[scrollbar_arrow_height];

private:
	Impl(const Impl&);
	Impl& operator=(const Impl&);

private:
	static int clamp(int value, int value_min, int value_max);

	void clear_error_message();
	void set_error_message(const char* message);
	void set_error_message(const char* prefix, const char* message);
	void set_omm_error_message();
	void set_error_message_from_sdl(const char* prefix);
	void font_decode_bitmap(const unsigned char* bitmap, Uint32* pixels);
	bool font_atlas_create(Uint32 sdl_texture_format);
	bool initialize_controls();

	static int to_nearest_even(int x);
	static void deflate(SDL_Rect& rect, int padding);

	static int utf8_decode(const char* chars, int char_count, int& code_point);
	static int utf8_encode(int code_point, char* chars, int char_count);

	static Uint32 subtract_ticks(Uint32 a, Uint32 b);
	static void clear_input_state(InputState& input_state);
	void handle_primary_button_state(Uint32 ticks, bool is_pressed);
	void handle_window_event(const SDL_WindowEvent& sdl_event);
	void handle_mouse_button_event(const SDL_MouseButtonEvent& sdl_event);
	void handle_mouse_motion_event(const SDL_MouseMotionEvent& sdl_event);
	void handle_mouse_wheel_event(const SDL_MouseWheelEvent& sdl_event);
	void handle_keyboard_event(const SDL_KeyboardEvent& sdl_event);
	void handle_text_input_event(const SDL_TextInputEvent& sdl_event);
	void render_reset_clip_rect();
	void render_set_clip_rect(SDL_Rect rect);
	void render_enable_font_blending(bool is_enable);
	void render_enable_font_blending();
	void render_disable_font_blending();
	void render_set_draw_color(SDL_Color color);
	void render_set_font_color(SDL_Color color);
	void render_point(int x, int y);
	void render_rect(const SDL_Rect& rect);
	void render_fill_rect(const SDL_Rect& rect);
	void render_line(int x0, int y0, int x1, int y1);
	void render_border(SDL_Rect rect, int width);
	void render_copy_font_glyph(const SDL_Rect& src_rect, const SDL_Rect& dst_rect);
	void calculate_metrics();
	static void string_to_font_glyph_indices_clamped(const char* string, VectorU8& glyph_indices);
	void font_glyph_indices_to_zstring_clamped(const VectorU8& glyph_indices, Text& text);
	static int choose_font_glyph_index_for_code_point(int code_point);
	const FontGlyph& choose_font_glyph_for_code_point(int code_point) const;
	void button_render_text(const Button& button, const SDL_Rect& clip_rect);
	bool ui_error_label_initialize();
	void ui_error_label_lay_out();
	void ui_error_label_render();
	bool button_initialize(Button& button);
	void button_calculate_metrics();
	void button_lay_out(Button& button) const;
	void button_render(Button& button);
	void button_update_input_state(Button& button);

	bool edit_box_initialize(EditBox& edit_box);
	void edit_box_update_window_offset(EditBox& edit_box);
	void edit_box_update_cursor_by_mouse(EditBox& edit_box);
	void edit_box_reset_cursor_blinking();
	void edit_box_update_cursor_blinking(EditBox& edit_box);
	void edit_box_clear(EditBox& edit_box);
	void edit_box_render_text(EditBox& edit_box);
	void edit_box_render_text_cursor(EditBox& edit_box);
	void edit_box_render(EditBox& edit_box);
	void edit_box_do_left(EditBox& edit_box);
	void edit_box_do_right(EditBox& edit_box);
	void edit_box_do_home(EditBox& edit_box);
	void edit_box_do_end(EditBox& edit_box);
	void edit_box_insert_text(EditBox& edit_box);
	void edit_box_do_del(EditBox& edit_box);
	void edit_box_do_backspace(EditBox& edit_box);
	void edit_box_update_input_state(EditBox& edit_box);

	bool ui_copy_button_initialize();
	void ui_copy_button_lay_out();
	void ui_copy_button_render();
	void ui_copy_button_update_input_state();
	void ui_copy_button_clicked_callback();
	bool ui_clear_button_initialize();
	void ui_clear_button_lay_out();
	void ui_clear_button_render();
	void ui_clear_button_update_input_state();
	void ui_clear_button_clicked_callback();
	bool ui_quit_button_initialize();
	void ui_quit_button_lay_out();
	void ui_quit_button_render();
	void ui_quit_button_update_input_state();
	void ui_quit_button_clicked_callback();
	bool ui_input_line_initialize();
	void ui_input_line_lay_out();
	void ui_input_line_render();
	void ui_input_line_text_callback(const char* text);
	void ui_input_line_update_input_state();
	bool scrollbar_initialize(Scrollbar& scrollbar);
	void scrollbar_update(Scrollbar& scrollbar);
	void scrollbar_render_arrow(const SDL_Rect& button_rect, ScrollbarArrowDirection direction);
	void scrollbar_render(const Scrollbar& scrollbar);
	bool text_box_initialize(TextBox& text_box, int text_capacity);
	void text_box_update_scrollbar(TextBox& text_box);
	void text_box_lay_out(TextBox& text_box, SDL_Rect rect);
	void text_box_clear_text(TextBox& text_box);
	void text_box_markup(TextBox& text_box);
	void text_box_append_text(TextBox& text_box, const char* text);
	void text_box_render_text(
		const TextBox& text_box,
		int line_index,
		int top_offset);
	void text_box_render(const TextBox& text_box);
	void text_box_update_input_state(TextBox& text_box);
	bool ui_log_initialize();
	void ui_log_lay_out();
	void ui_log_update_input_state();
	void exclude_input_line_from_focusables();
	void lay_out_controls();
	int find_focused_control_index() const;
	void update_controls_dragging_scrollbar();
	void update_primary_button_repeating();
	void update_controls_keyboard_navigation();
	void update_controls_edit_box_blinking_cursor();
	void update_controls_input_state();
	void render_controls();
};

// --------------------------------------------------------------------------

const SDL_Color Syscon::Impl::color_window = {0xF0, 0xF0, 0xF0, 0xFF};

const SDL_Color Syscon::Impl::color_button_text = {0x00, 0x00, 0x00, 0xFF};
const SDL_Color Syscon::Impl::color_button_border = {0xAD, 0xAD, 0xAD, 0xFF};
const SDL_Color Syscon::Impl::color_button_background = {0xE1, 0xE1, 0xE1, 0xFF};
const SDL_Color Syscon::Impl::color_button_focused = {0x00, 0x78, 0xD7, 0xFF};
const SDL_Color Syscon::Impl::color_button_highlighted = {0xE5, 0xF1, 0xFB, 0xFF};
const SDL_Color Syscon::Impl::color_button_pressed = {0xCC, 0xE4, 0xF7, 0xFF};

const SDL_Color Syscon::Impl::color_scrollbar_track = {0xF0, 0xF0, 0xF0, 0xFF};
const SDL_Color Syscon::Impl::color_scrollbar_thumb = {0xCD, 0xCD, 0xCD, 0xFF};
const SDL_Color Syscon::Impl::color_scrollbar_thumb_highlighted = {0xC0, 0xC0, 0xC0, 0xFF};
const SDL_Color Syscon::Impl::color_scrollbar_thumb_pressed = {0x60, 0x60, 0x60, 0xFF};
const SDL_Color Syscon::Impl::color_scrollbar_button = {0xF0, 0xF0, 0xF0, 0xFF};
const SDL_Color Syscon::Impl::color_scrollbar_button_highlighted = {0xDA, 0xDA, 0xDA, 0xFF};
const SDL_Color Syscon::Impl::color_scrollbar_button_pressed = {0x60, 0x60, 0x60, 0xFF};
const SDL_Color Syscon::Impl::color_scrollbar_arrow = {0x60, 0x60, 0x60, 0xFF};
const SDL_Color Syscon::Impl::color_scrollbar_arrow_highlighted = {0x00, 0x00, 0x00, 0xFF};
const SDL_Color Syscon::Impl::color_scrollbar_arrow_pressed = {0xFF, 0xFF, 0xFF, 0xFF};

const SDL_Color Syscon::Impl::color_text_box_border = {0x7A, 0x7A, 0x7A, 0xFF};
const SDL_Color Syscon::Impl::color_text_box_border_highlighted = {0x17, 0x17, 0x17, 0xFF};
const SDL_Color Syscon::Impl::color_text_box_border_focused = {0x00, 0x78, 0xD7, 0xFF};
const SDL_Color Syscon::Impl::color_text_box_background = {0xFF, 0xFF, 0xFF, 0xFF};
const SDL_Color Syscon::Impl::color_text_box_text = {0x00, 0x00, 0x00, 0xFF};

const SDL_Color Syscon::Impl::color_edit_box_border = {0x7A, 0x7A, 0x7A, 0xFF};
const SDL_Color Syscon::Impl::color_edit_box_border_highlighted = {0x17, 0x17, 0x17, 0xFF};
const SDL_Color Syscon::Impl::color_edit_box_border_focused = {0x00, 0x78, 0xD7, 0xFF};
const SDL_Color Syscon::Impl::color_edit_box_background = {0xFF, 0xFF, 0xFF, 0xFF};
const SDL_Color Syscon::Impl::color_edit_box_text = {0x00, 0x00, 0x00, 0xFF};

const SDL_Color Syscon::Impl::color_ui_error_label_text = {0xFF, 0x00, 0x00, 0xFF};

// --------------------------------------------------------------------------

const unsigned short Syscon::Impl::scrollbar_up_arrow_bitmap[scrollbar_arrow_height] =
{
	        // MSB               LSB
	        // |FEDC|BA98|7654|3210|
	0x0020, // |    |    |  X |    |
	0x0070, // |    |    | XXX|    |
	0x00F8, // |    |    |XXXX|X   |
	0x01FC, // |    |   X|XXXX|XX  |
	0x03DE, // |    |  XX|XX X|XXX |
	0x078F, // |    | XXX|X   |XXXX|
	0x0707, // |    | XXX|    | XXX|
	0x0603, // |    | XX |    |  XX|
	0x0401  // |    | X  |    |   X|
};

// --------------------------------------------------------------------------

Syscon::Impl::Impl()
	:
	is_initialized_(),
	is_sdl_at_least_2_0_4(),
	is_error_mode_(),
	has_mouse_focus_(),
	has_keyboard_focus_(),
	callback_(),
	sdl_subsystem_(),
	sdl_window_uptr_(),
	sdl_renderer_uptr_(),
	sdl_window_id_(),
	scale_factor_(),
	window_width_(),
	window_height_(),
	font_glyph_width_(),
	font_glyph_height_(),
	margin_(),
	button_padding_(),
	button_height_(),
	button_width_(),
	font_atlas_(),
	font_glyphs_(),
	mouse_position_(),
	ui_error_label_(),
	ui_copy_button_(),
	ui_clear_button_(),
	ui_quit_button_(),
	ui_log_(),
	ui_input_line_(),
	focused_control_(),
	focusable_controls_count_(),
	dragging_scrollbar_(),
	dragging_scrollbar_anchor_(),
	edit_box_cursor_is_visible_(),
	edit_box_cursor_last_ticks_(),
	focusable_controls_(),
	old_input_state_(),
	input_state_(),
	error_message_()
{}

Syscon::Impl::~Impl()
{
	terminate();
}

bool Syscon::Impl::is_initialized() const
{
	return is_initialized_;
}

const char* Syscon::Impl::get_error_message() const
{
	return error_message_.get_data();
}

const char* Syscon::Impl::get_text() const
{
	return ui_log_.text.get_data();
}

bool Syscon::Impl::initialize(Callback callback)
{
	set_error_message("Not initialized.");

	// Initialize SDL video subsystem.
	//
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
	{
		set_error_message_from_sdl("SDL_InitSubSystem");
		return false;
	}

	sdl_subsystem_.reset(SDL_INIT_VIDEO);

	SDL_version sdl_version_rt;
	SDL_GetVersion(&sdl_version_rt);

	if (sdl_version_rt.major != 2)
	{
		set_error_message_from_sdl("Unsupported SDL major runtime version.");
		return false;
	}

	is_sdl_at_least_2_0_4 = sdl_version_rt.minor > 0 || sdl_version_rt.patch >= 4;

	calculate_metrics();

	// Create SDL window.
	//
	SDL_Window* sdl_window = SDL_CreateWindow(
		"Console",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		window_width_,
		window_height_,
		SDL_WINDOW_RESIZABLE);
	
	if (sdl_window == NULL)
	{
		set_error_message_from_sdl("SDL_CreateWindow");
		return false;
	}

	sdl_window_uptr_.reset(sdl_window);

	// Create software renderer.
	//
	SDL_Renderer* sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_SOFTWARE);

	if (sdl_renderer == NULL)
	{
		set_error_message_from_sdl("SDL_CreateRenderer");
		return false;
	}

	sdl_renderer_uptr_.reset(sdl_renderer);

	// Get an identifier of the window.
	//
	sdl_window_id_ = SDL_GetWindowID(sdl_window);

	if (sdl_window_id_ == 0)
	{
		set_error_message_from_sdl("SDL_GetWindowID");
		return false;
	}

	// Get renderer info.
	//
	SDL_RendererInfo sdl_renderer_info;

	if (SDL_GetRendererInfo(sdl_renderer, &sdl_renderer_info) != 0)
	{
		set_error_message_from_sdl("SDL_GetRendererInfo");
		return false;
	}

	// Validate max texture size.
	//
	const int max_dimenstion = std::max(font_atlas_width, font_atlas_height);

	if ((sdl_renderer_info.max_texture_width != 0 && sdl_renderer_info.max_texture_width < max_dimenstion) ||
		(sdl_renderer_info.max_texture_height != 0 && sdl_renderer_info.max_texture_height < max_dimenstion))
	{
		set_error_message("Renderer's texture size too small.");
		return false;
	}

	// Validate texture format.
	//
	Uint32 sdl_texture_format = SDL_PIXELFORMAT_UNKNOWN;

	for (Uint32 i = 0; i < sdl_renderer_info.num_texture_formats; ++i)
	{
		const Uint32 format = sdl_renderer_info.texture_formats[i];

		if (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_PACKED32 &&
			SDL_PIXELLAYOUT(format) == SDL_PACKEDLAYOUT_8888 &&
			SDL_ISPIXELFORMAT_ALPHA(format))
		{
			sdl_texture_format = format;
			break;
		}
	}

	// Initialize input state.
	//
	input_state_.glyph_indices.reserve(SDL_TEXTINPUTEVENT_TEXT_SIZE);
	old_input_state_.glyph_indices.reserve(SDL_TEXTINPUTEVENT_TEXT_SIZE);
	
	// Initialize the rest.
	//
	if (!font_atlas_create(sdl_texture_format))
	{
		return false;
	}

	if (!initialize_controls())
	{
		return false;
	}

	lay_out_controls();

	callback_ = callback;

	// Commit.
	//
	clear_error_message();
	is_initialized_ = true;
	return true;
}

void Syscon::Impl::terminate()
{
	font_atlas_.reset();
	sdl_renderer_uptr_.reset();
	sdl_window_uptr_.reset();
	sdl_subsystem_.reset();
}

void Syscon::Impl::set_title(const char* title)
{
	SDL_SetWindowTitle(sdl_window_uptr_.get(), title);
}

void Syscon::Impl::show(ShowMode mode)
{
	switch (mode)
	{
		case show_mode_normal:
			SDL_ShowWindow(sdl_window_uptr_.get());
			break;

		case show_mode_hidden:
			SDL_HideWindow(sdl_window_uptr_.get());
			break;

		case show_mode_minimized:
			SDL_MinimizeWindow(sdl_window_uptr_.get());
			break;
	}
}

void Syscon::Impl::clear_text()
{
	text_box_clear_text(ui_log_);
}

void Syscon::Impl::append_text(const char* text)
{
	text_box_append_text(ui_log_, text);
}

void Syscon::Impl::set_error_text(const char* text)
{
	is_error_mode_ = true;
	lay_out_controls();

	text_box_clear_text(ui_error_label_);

	if (text == NULL || text[0] == '\0')
	{
		text = "Generic failure.";
	}

	text_box_append_text(ui_error_label_, text);
}

void Syscon::Impl::update()
{
	update_controls_input_state();

	if ((SDL_GetWindowFlags(sdl_window_uptr_.get()) & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED)) == 0)
	{
		render_reset_clip_rect();
		render_set_draw_color(color_window);
		SDL_RenderClear(sdl_renderer_uptr_.get());
		render_controls();
		SDL_RenderPresent(sdl_renderer_uptr_.get());
	}
}

void Syscon::Impl::handle_event(const SDL_Event& sdl_event)
{
	switch (sdl_event.type)
	{
		case SDL_QUIT:
			if (callback_ != NULL)
			{
				const CallbackParam param = CallbackParam(CallbackQuitTag());
				callback_(param);
			}

			break;

		case SDL_WINDOWEVENT:
			handle_window_event(sdl_event.window);
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			handle_mouse_button_event(sdl_event.button);
			break;

		case SDL_MOUSEMOTION:
			handle_mouse_motion_event(sdl_event.motion);
			break;

		case SDL_MOUSEWHEEL:
			handle_mouse_wheel_event(sdl_event.wheel);
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			handle_keyboard_event(sdl_event.key);
			break;

		case SDL_TEXTINPUT:
			handle_text_input_event(sdl_event.text);
			break;
	}

	update_controls_input_state();
}

void Syscon::Impl::run()
{
	is_error_mode_ = true;
	exclude_input_line_from_focusables();
	lay_out_controls();

	SDL_Event sdl_event;
	bool is_quit = false;
	Uint32 last_ticks = SDL_GetTicks();

	while (!is_quit)
	{
		while (SDL_PollEvent(&sdl_event) != 0)
		{
			if (sdl_event.type == SDL_QUIT)
			{
				is_quit = true;
			}

			handle_event(sdl_event);
		}

		update();

		const Uint32 ticks = SDL_GetTicks();
		const Uint32 duration_ms = subtract_ticks(ticks, last_ticks);
		last_ticks = ticks;

		if (duration_ms < 15)
		{
			SDL_Delay(20);
		}
	}
}

int Syscon::Impl::clamp(int value, int value_min, int value_max)
{
	assert(value_min <= value_max);
	return value < value_min ? value_min : (value > value_max ? value_max : value);
}

void Syscon::Impl::clear_error_message()
{
	error_message_[0] = '\0';
}

void Syscon::Impl::set_error_message(const char* message)
{
	set_error_message(NULL, message);
}

void Syscon::Impl::set_error_message(const char* prefix, const char* message)
{
	int offset = 0;

	if (prefix != NULL && offset < error_message_max_length)
	{
		error_message_[offset] = '[';
		++offset;
	}

	if (prefix != NULL)
	{
		const int prefix_length = std::min(static_cast<int>(strlen(prefix)), error_message_max_length - offset);
		memcpy(&error_message_[offset], prefix, static_cast<size_t>(prefix_length));
		offset += prefix_length;
	}

	if (prefix != NULL)
	{
		if (offset < error_message_max_length)
		{
			error_message_[offset] = ']';
			++offset;
		}

		if (offset < error_message_max_length)
		{
			error_message_[offset] = ' ';
			++offset;
		}
	}

	if (message != NULL)
	{
		const int message_length = std::min(static_cast<int>(strlen(message)), error_message_max_length - offset);
		memcpy(&error_message_[offset], message, static_cast<size_t>(message_length));
		offset += message_length;
	}

	error_message_[offset] = '\0';
}

void Syscon::Impl::set_omm_error_message()
{
	set_error_message("Out of memory.");
}

void Syscon::Impl::set_error_message_from_sdl(const char* prefix)
{
	set_error_message(prefix, SDL_GetError());
}

void Syscon::Impl::font_decode_bitmap(const unsigned char* bitmap, Uint32* pixels)
{
	Uint32* pixel_line = pixels;

	for (int i_byte = 0; i_byte < 16; ++i_byte)
	{
		const unsigned int line_bitmap = bitmap[i_byte];

		for (int i_bit_index = 0; i_bit_index < 8; ++i_bit_index)
		{
			const bool bit = (line_bitmap & (1 << (7 - i_bit_index))) != 0;
			const Uint32 pixel = bit * 0xFFFFFFFFU;
			pixel_line[i_bit_index] = pixel;
		}

		pixel_line += font_atlas_width;
	}
}

bool Syscon::Impl::font_atlas_create(Uint32 sdl_texture_format)
{
	typedef VectorTrivial<Uint32> FontAtlasPixels;

	FontAtlasPixels font_atlas_pixels_;

	if (!font_atlas_pixels_.resize_uninitialized(font_atlas_area))
	{
		set_error_message("Out of memory.");
	}

	int glyph_cell_x = 0;
	int glyph_cell_y = 0;
	int missing_glyph_x = 0;
	int missing_glyph_y = 0;
	const unsigned char* bitmap = SysconFont16x8::bitmap;

	for (int i_glyph = 0; i_glyph < font_glyph_count; ++i_glyph)
	{
		const int code_point = font_first_code_point + i_glyph;
		FontGlyph& glyph = font_glyphs_[code_point];

		if (!SysconFont16x8::has_code_point(code_point))
		{
			continue;
		}

		const int atlas_x = glyph_cell_x * font_glyph_width;
		const int atlas_y = glyph_cell_y * font_glyph_height;

		if (code_point == font_missing_code_point)
		{
			missing_glyph_x = atlas_x;
			missing_glyph_y = atlas_y;
		}

		glyph.atlas_x = static_cast<unsigned char>(atlas_x);
		glyph.atlas_y = static_cast<unsigned char>(atlas_y);

		const int pixels_index = atlas_y * font_atlas_width + atlas_x;
		Uint32* glyph_pixels = &font_atlas_pixels_[pixels_index];
		font_decode_bitmap(bitmap, glyph_pixels);

		glyph_cell_x += 1;

		if (glyph_cell_x == font_atlas_glyphs_per_width)
		{
			glyph_cell_x = 0;
			++glyph_cell_y;
		}

		bitmap += SysconFont16x8::bytes_per_glyph;
	}

	// Point all unmapped glyphs to the special one.
	for (int code_point = 0; code_point < font_max_glyphs; ++code_point)
	{
		if (SysconFont16x8::has_code_point(code_point))
		{
			continue;
		}

		FontGlyph& glyph = font_glyphs_[code_point];
		glyph.atlas_x = static_cast<unsigned char>(missing_glyph_x);
		glyph.atlas_y = static_cast<unsigned char>(missing_glyph_y);
	}

	if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") != SDL_TRUE)
	{
		set_error_message_from_sdl("SDL_SetHint");
		return false;
	}

	SDL_Texture* sdl_texture = SDL_CreateTexture(
		sdl_renderer_uptr_.get(),
		sdl_texture_format,
		SDL_TEXTUREACCESS_STATIC,
		256,
		128);

	if (sdl_texture == NULL)
	{
		set_error_message_from_sdl("SDL_CreateTexture");
		return false;
	}

	font_atlas_.reset(sdl_texture);

	if (SDL_UpdateTexture(sdl_texture, NULL, font_atlas_pixels_.get_data(), 4 * font_atlas_width) != 0)
	{
		set_error_message_from_sdl("SDL_UpdateTexture");
		return false;
	}

	if (SDL_SetTextureColorMod(sdl_texture, 0xFF, 0xFF, 0xFF) != 0)
	{
		set_error_message_from_sdl("SDL_SetTextureColorMod");
		return false;
	}

	return true;
}

bool Syscon::Impl::initialize_controls()
{
	mouse_position_.x = -1;
	mouse_position_.y = -1;

	if (!ui_error_label_initialize() ||
		!ui_copy_button_initialize() ||
		!ui_clear_button_initialize() ||
		!ui_quit_button_initialize() ||
		!ui_input_line_initialize() ||
		!ui_log_initialize())
	{
		return false;
	}

	focusable_controls_count_ = 0;
	focusable_controls_[focusable_controls_count_++] = &ui_input_line_;
	focusable_controls_[focusable_controls_count_++] = &ui_copy_button_;
	focusable_controls_[focusable_controls_count_++] = &ui_clear_button_;
	focusable_controls_[focusable_controls_count_++] = &ui_quit_button_;
	focusable_controls_[focusable_controls_count_++] = &ui_log_;
	focused_control_ = &ui_input_line_;
	return true;
}

int Syscon::Impl::to_nearest_even(int x)
{
	const unsigned int mask_uint = (~0U) ^ 1U;
	const int mask = static_cast<int>(mask_uint);
	return x & mask;
}

void Syscon::Impl::deflate(SDL_Rect& rect, int padding)
{
	rect.x += padding;
	rect.y += padding;
	rect.w -= 2 * padding;
	rect.h -= 2 * padding;
}

int Syscon::Impl::utf8_decode(const char* chars, int char_count, int& code_point)
{
	code_point = unicode_replacement_code_point;

	if (char_count <= 0)
	{
		// Empty input.
		return 0;
	}

	const unsigned char* bytes = reinterpret_cast<const unsigned char*>(chars);
	const int byte_1 = bytes[0];
	int byte_count = 0;
	int new_code_point = 0;
	int first_code_point = 0;
	int last_code_point = 0;

	if ((byte_1 & 0x80) == 0) // 1xxx'xxxx
	{
		byte_count = 1;
		new_code_point = byte_1;
		first_code_point = 0;
		last_code_point = 0x007F;
	}
	else if ((byte_1 & 0xE0) == 0xC0) // 110x'xxxx
	{
		byte_count = 2;
		new_code_point = byte_1 & 0x1F;
		first_code_point = 0x0080;
		last_code_point = 0x07FF;
	}
	else if ((byte_1 & 0xF0) == 0xE0) // 1110'xxxx
	{
		byte_count = 3;
		new_code_point = byte_1 & 0x0F;
		first_code_point = 0x0800;
		last_code_point = 0xFFFF;
	}
	else if ((byte_1 & 0xF8) == 0xF0) // 1111'0xxx
	{
		byte_count = 4;
		new_code_point = byte_1 & 0x07;
		first_code_point = 0x010000;
		last_code_point = 0x10FFFF;
	}

	if (byte_count == 0 || byte_count > char_count)
	{
		// Invalid first byte, or not enough of input bytes.
		return 1;
	}

	for (int i_byte = 1; i_byte < byte_count; ++i_byte)
	{
		const int byte_n = bytes[i_byte]; // 10xx'xxxx

		if ((byte_n & 0xC0) != 0x80)
		{
			// Invalid n-th byte.
			return 1;
		}

		new_code_point <<= 6;
		new_code_point |= bytes[i_byte] & 0x3F;
	}

	if (new_code_point < first_code_point || new_code_point > last_code_point)
	{
		// Overlong encoding, or code point is out of range.
		return 1;
	}

	if (new_code_point >= unicode_min_surrogate && new_code_point <= unicode_max_surrogate)
	{
		// Surrogate.
		return 1;
	}

	code_point = new_code_point;
	return byte_count;
}

int Syscon::Impl::utf8_encode(int code_point, char* chars, int char_count)
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(chars);

	if (code_point >= utf8_first_code_point_1 && code_point <= utf8_last_code_point_2)
	{
		if (char_count < 1)
		{
			return 0;
		}

		bytes[0] = static_cast<unsigned char>(code_point);
		return 1;
	}
	else if (code_point >= utf8_first_code_point_2 && code_point <= utf8_last_code_point_2)
	{
		if (char_count < 2)
		{
			return 0;
		}

		bytes[0] = static_cast<unsigned char>(0xC0 | (code_point >> 6));
		bytes[1] = static_cast<unsigned char>(code_point & 0x3F);
		return 2;
	}
	else if (code_point >= utf8_first_code_point_3 && code_point <= utf8_last_code_point_3)
	{
		if (char_count < 3)
		{
			return 0;
		}

		bytes[0] = static_cast<unsigned char>(0xE0 | (code_point >> 12));
		bytes[1] = static_cast<unsigned char>((code_point >> 6) & 0x3F);
		bytes[2] = static_cast<unsigned char>(code_point & 0x3F);
		return 3;
	}
	else if (code_point >= utf8_first_code_point_4 && code_point <= utf8_last_code_point_4)
	{
		if (char_count < 4)
		{
			return 0;
		}

		bytes[0] = static_cast<unsigned char>(0xF0 | (code_point >> 18));
		bytes[1] = static_cast<unsigned char>((code_point >> 12) & 0x3F);
		bytes[2] = static_cast<unsigned char>((code_point >> 6) & 0x3F);
		bytes[3] = static_cast<unsigned char>(code_point & 0x3F);
		return 4;
	}
	else
	{
		return 0;
	}
}

Uint32 Syscon::Impl::subtract_ticks(Uint32 a, Uint32 b)
{
	return (a - b) + (a < b ? 0xFFFFFFFFU : 0);
}

void Syscon::Impl::clear_input_state(InputState& input_state)
{
	input_state.is_primary_button_pressed = false;
	input_state.is_primary_button_repeated = false;
	input_state.primary_button_last_repeat_ticks = 0;
	input_state.mouse_wheel_v_delta = 0;
	input_state.pressed_keys.fill(false);
	input_state.repeated_keys.fill(false);
	input_state.glyph_indices.clear();
}

void Syscon::Impl::handle_primary_button_state(Uint32 ticks, bool is_pressed)
{
	input_state_.is_primary_button_pressed = is_pressed;

	if (!old_input_state_.is_primary_button_pressed &&
		input_state_.is_primary_button_pressed)
	{
		input_state_.is_primary_button_repeated = true;
		input_state_.primary_button_last_repeat_ticks = ticks;
	}
}

void Syscon::Impl::handle_window_event(const SDL_WindowEvent& sdl_event)
{
	if (sdl_event.windowID != sdl_window_id_)
	{
		return;
	}

	switch (sdl_event.event)
	{
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			has_keyboard_focus_ = true;
			break;

		case SDL_WINDOWEVENT_FOCUS_LOST:
			has_keyboard_focus_ = false;
			break;

		case SDL_WINDOWEVENT_ENTER:
			has_mouse_focus_ = true;
			break;

		case SDL_WINDOWEVENT_LEAVE:
			has_mouse_focus_ = false;
			break;

		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			SDL_GetWindowSize(sdl_window_uptr_.get(), &window_width_, &window_height_);
			lay_out_controls();
			break;

		case SDL_WINDOWEVENT_HIDDEN:
			if (callback_ != NULL)
			{
				const CallbackParam param(show_mode_hidden);
				callback_(param);
			}

			break;

		case SDL_WINDOWEVENT_SHOWN:
			if (callback_ != NULL)
			{
				const CallbackParam param(show_mode_normal);
				callback_(param);
			}

			break;

		case SDL_WINDOWEVENT_MINIMIZED:
			if (callback_ != NULL)
			{
				const CallbackParam param(show_mode_minimized);
				callback_(param);
			}

			break;
	}
}

void Syscon::Impl::handle_mouse_button_event(const SDL_MouseButtonEvent& sdl_event)
{
	if (sdl_event.windowID != sdl_window_id_)
	{
		return;
	}

	mouse_position_.x = sdl_event.x;
	mouse_position_.y = sdl_event.y;

	if (sdl_event.button == SDL_BUTTON_LEFT)
	{
		handle_primary_button_state(sdl_event.timestamp, sdl_event.state == SDL_PRESSED);
	}
}

void Syscon::Impl::handle_mouse_motion_event(const SDL_MouseMotionEvent& sdl_event)
{
	if (sdl_event.windowID != sdl_window_id_)
	{
		return;
	}

	mouse_position_.x = sdl_event.x;
	mouse_position_.y = sdl_event.y;
}

void Syscon::Impl::handle_mouse_wheel_event(const SDL_MouseWheelEvent& sdl_event)
{
	if (sdl_event.windowID != sdl_window_id_)
	{
		return;
	}

	input_state_.mouse_wheel_v_delta = sdl_event.y;

#if SDL_VERSION_ATLEAST(2, 0, 4)
	if (is_sdl_at_least_2_0_4)
	{
		switch (sdl_event.direction)
		{
			case SDL_MOUSEWHEEL_FLIPPED:
				input_state_.mouse_wheel_v_delta = -input_state_.mouse_wheel_v_delta;
				break;
		}
	}
#endif // SDL_VERSION_ATLEAST(2, 0, 4)
}

void Syscon::Impl::handle_keyboard_event(const SDL_KeyboardEvent& sdl_event)
{
	Keyindex keyindex = keyindex_none;

	switch (sdl_event.keysym.sym)
	{
		case SDLK_LCTRL:     keyindex = keyindex_lctrl;     break;
		case SDLK_RCTRL:     keyindex = keyindex_rctrl;     break;
		case SDLK_LSHIFT:    keyindex = keyindex_lshift;    break;
		case SDLK_RSHIFT:    keyindex = keyindex_rshift;    break;

		case SDLK_TAB:       keyindex = keyindex_tab;       break;
		case SDLK_BACKSPACE: keyindex = keyindex_backspace; break;
		case SDLK_SPACE:     keyindex = keyindex_space;     break;
		case SDLK_RETURN:    keyindex = keyindex_enter;     break;
		case SDLK_INSERT:    keyindex = keyindex_insert;    break;
		case SDLK_DELETE:    keyindex = keyindex_delete;    break;
		case SDLK_HOME:      keyindex = keyindex_home;      break;
		case SDLK_END:       keyindex = keyindex_end;       break;
		case SDLK_PAGEUP:    keyindex = keyindex_page_up;   break;
		case SDLK_PAGEDOWN:  keyindex = keyindex_page_down; break;
		case SDLK_LEFT:      keyindex = keyindex_left;      break;
		case SDLK_RIGHT:     keyindex = keyindex_right;     break;
		case SDLK_UP:        keyindex = keyindex_up;        break;
		case SDLK_DOWN:      keyindex = keyindex_down;      break;
	}

	if (keyindex == keyindex_none)
	{
		return;
	}

	const bool is_pressed = sdl_event.state == SDL_PRESSED;
	input_state_.pressed_keys[keyindex] = is_pressed;
	input_state_.repeated_keys[keyindex] = is_pressed;
}

void Syscon::Impl::handle_text_input_event(const SDL_TextInputEvent& sdl_event)
{
	string_to_font_glyph_indices_clamped(sdl_event.text, input_state_.glyph_indices);
}

void Syscon::Impl::render_reset_clip_rect()
{
	SDL_RenderSetClipRect(sdl_renderer_uptr_.get(), NULL);
}

void Syscon::Impl::render_set_clip_rect(SDL_Rect rect)
{
	SDL_RenderSetClipRect(sdl_renderer_uptr_.get(), &rect);
}

void Syscon::Impl::render_enable_font_blending(bool is_enable)
{
	const SDL_BlendMode sdl_blend_mode = is_enable ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE;
	SDL_SetTextureBlendMode(font_atlas_.get(), sdl_blend_mode);
}

void Syscon::Impl::render_enable_font_blending()
{
	render_enable_font_blending(true);
}

void Syscon::Impl::render_disable_font_blending()
{
	render_enable_font_blending(false);
}

void Syscon::Impl::render_set_draw_color(SDL_Color color)
{
	SDL_SetRenderDrawColor(sdl_renderer_uptr_.get(), color.r, color.g, color.b, 0xFF);
}

void Syscon::Impl::render_set_font_color(SDL_Color color)
{
	SDL_SetTextureColorMod(font_atlas_.get(), color.r, color.g, color.b);
}

void Syscon::Impl::render_point(int x, int y)
{
	SDL_RenderDrawPoint(sdl_renderer_uptr_.get(), x, y);
}

void Syscon::Impl::render_rect(const SDL_Rect& rect)
{
	SDL_RenderDrawRect(sdl_renderer_uptr_.get(), &rect);
}

void Syscon::Impl::render_fill_rect(const SDL_Rect& rect)
{
	SDL_RenderFillRect(sdl_renderer_uptr_.get(), &rect);
}

void Syscon::Impl::render_line(int x0, int y0, int x1, int y1)
{
	SDL_RenderDrawLine(sdl_renderer_uptr_.get(), x0, y0, x1, y1);
}

void Syscon::Impl::render_border(SDL_Rect rect, int width)
{
	for (int i = 0; i < width; ++i)
	{
		render_rect(rect);
		deflate(rect, 1);
	}
}

void Syscon::Impl::render_copy_font_glyph(const SDL_Rect& src_rect, const SDL_Rect& dst_rect)
{
	SDL_RenderCopy(sdl_renderer_uptr_.get(), font_atlas_.get(), &src_rect, &dst_rect);
}

void Syscon::Impl::calculate_metrics()
{
	scale_factor_ = 1;
	window_width_ = window_min_width;
	window_height_ = window_min_height;
	font_glyph_width_ = font_glyph_width;
	font_glyph_height_ = font_glyph_height;
	margin_ = default_margin;

	int display_index = -1;

	if (sdl_window_uptr_.get() != NULL)
	{
		display_index = SDL_GetWindowDisplayIndex(sdl_window_uptr_.get());
		assert(display_index >= 0);
	}

	if (display_index < 0)
	{
		display_index = 0;
	}

	SDL_DisplayMode sdl_mode;

	if (SDL_GetCurrentDisplayMode(display_index, &sdl_mode) == 0)
	{
		const int min_dimension = std::min(sdl_mode.h, sdl_mode.w);
		scale_factor_ = std::max(0.75 * min_dimension / window_min_height, 1.0);
	}

	const double font_scale_factor = std::max(scale_factor_ * 0.75, 1.0);

	window_width_ = to_nearest_even(static_cast<int>(scale_factor_ * window_min_width + 0.5));
	window_height_ = to_nearest_even(static_cast<int>(scale_factor_ * window_min_height + 0.5));
	margin_ = static_cast<int>(default_margin * scale_factor_ + 0.5);

	font_glyph_width_ = static_cast<int>(font_glyph_width * font_scale_factor + 0.5);
	font_glyph_height_ = static_cast<int>(font_glyph_height * font_scale_factor + 0.5);

	button_calculate_metrics();
}

void Syscon::Impl::string_to_font_glyph_indices_clamped(const char* string, VectorU8& glyph_indices)
{
	const int u8_length = static_cast<int>(strlen(string));
	const int capacity = glyph_indices.get_capacity();

	if (capacity <= 0 || !glyph_indices.resize_uninitialized(capacity))
	{
		return;
	}

	int cp_index = 0;

	for (int i_u8 = 0; i_u8 < u8_length;)
	{
		int code_point;
		const int u8_byte_count = utf8_decode(&string[i_u8], u8_length - i_u8, code_point);

		if (u8_byte_count == 0 || cp_index == capacity)
		{
			break;
		}

		const int glyph_index = choose_font_glyph_index_for_code_point(code_point);
		glyph_indices[cp_index++] = static_cast<unsigned char>(glyph_index);
		i_u8 += u8_byte_count;
	}

	glyph_indices.resize_uninitialized(cp_index);
}

void Syscon::Impl::font_glyph_indices_to_zstring_clamped(const VectorU8& glyph_indices, Text& text)
{
	const int string_capacity = text.get_capacity();

	if (string_capacity == 0)
	{
		return;
	}

	const unsigned char* indices = glyph_indices.get_data();
	const int index_count = glyph_indices.get_size();
	char* string = text.get_data();

	int u8_count = 0;

	for (int i_index = 0; i_index < index_count; ++i_index)
	{
		const int written_count = utf8_encode(indices[i_index], &string[u8_count], string_capacity - 1 - u8_count);

		if (written_count == 0)
		{
			break;
		}

		u8_count += written_count;
	}

	string[u8_count] = '\0';
	text.resize_uninitialized(u8_count);
}

int Syscon::Impl::choose_font_glyph_index_for_code_point(int code_point)
{
	return SysconFont16x8::has_code_point(code_point) ? code_point : font_missing_code_point;
}

const Syscon::Impl::FontGlyph& Syscon::Impl::choose_font_glyph_for_code_point(int code_point) const
{
	const int glyph_index = choose_font_glyph_index_for_code_point(code_point);
	return font_glyphs_[glyph_index];
}

void Syscon::Impl::button_render_text(const Button& button, const SDL_Rect& clip_rect)
{
	if (button.glyph_indices.is_empty() ||
		button.rect.w <= 0 || button.rect.h <= 0)
	{
		return;
	}

	const VectorU8& glyph_indices = button.glyph_indices;

	const int text_width = button.glyph_indices.get_size() * font_glyph_width_;
	const int target_x_min = clip_rect.x + (clip_rect.w - text_width) / 2;
	const int target_x_max = target_x_min + text_width;
	const int target_y_min = clip_rect.y + (clip_rect.h - font_glyph_height_) / 2;

	SDL_Rect glyph_atlas_rect = SDL_Rect();
	glyph_atlas_rect.w = font_glyph_width;
	glyph_atlas_rect.h = font_glyph_height;

	SDL_Rect glyph_target_rect = SDL_Rect();
	glyph_target_rect.x = target_x_min;
	glyph_target_rect.y = target_y_min;
	glyph_target_rect.w = font_glyph_width_;
	glyph_target_rect.h = font_glyph_height_;

	render_set_clip_rect(clip_rect);
	render_set_font_color(color_button_text);
	render_enable_font_blending();

	for (int i_glyph_index = 0, n_glyph_index = glyph_indices.get_size(); i_glyph_index < n_glyph_index; ++i_glyph_index)
	{
		if (glyph_target_rect.x >= target_x_max)
		{
			break;
		}

		if (glyph_target_rect.x + glyph_target_rect.w > target_x_min)
		{
			const unsigned char glyph_index = glyph_indices[i_glyph_index];
			const FontGlyph& glyph = font_glyphs_[glyph_index];
			glyph_atlas_rect.x = glyph.atlas_x;
			glyph_atlas_rect.y = glyph.atlas_y;

			render_copy_font_glyph(glyph_atlas_rect, glyph_target_rect);
		}

		glyph_target_rect.x += font_glyph_width_;
	}

	render_disable_font_blending();
}

bool Syscon::Impl::ui_error_label_initialize()
{
	TextBox& text_box = ui_error_label_;

	if (!text_box_initialize(text_box, error_label_max_text_length))
	{
		return false;
	}

	text_box.scrollbar.is_visible = false;
	text_box.text_color = color_ui_error_label_text;
	return true;
}

void Syscon::Impl::ui_error_label_lay_out()
{
	TextBox& text_box = ui_error_label_;

	SDL_Rect rect = SDL_Rect();

	const int height =
		error_label_line_count * font_glyph_height_ +
		2 * (default_border_width + default_text_box_padding);

	rect.x = margin_;
	rect.y = margin_;
	rect.w = window_width_ - 2 * margin_;
	rect.h = height;

	text_box_lay_out(text_box, rect);
}

void Syscon::Impl::ui_error_label_render()
{
	text_box_render(ui_error_label_);
}

bool Syscon::Impl::button_initialize(Button& button)
{
	button.is_highlighted = false;
	button.is_pressed = false;
	return button.glyph_indices.reserve(default_button_width_in_glyphs);
}

void Syscon::Impl::button_calculate_metrics()
{
	button_padding_ = static_cast<int>(default_button_padding * scale_factor_ + 0.5);
	button_height_ = font_glyph_height_ + 2 * (default_border_width + button_padding_);
	button_width_ = default_button_width_in_glyphs * font_glyph_width_;
}

void Syscon::Impl::button_lay_out(Button& button) const
{
	button.rect.x = margin_;
	button.rect.y = window_height_ - margin_ - button_height_;
	button.rect.w = default_button_width_in_glyphs * font_glyph_width_;
	button.rect.h = button_height_;
}

void Syscon::Impl::button_render(Button& button)
{
	const bool is_focused = focused_control_ == &button;
	render_reset_clip_rect();

	SDL_Rect rect = button.rect;
	const int extra_border_width = is_focused ? default_extra_border_width : 0;
	const int border_width = default_border_width + extra_border_width;

	render_set_draw_color(is_focused || button.is_highlighted ? color_button_focused : color_button_border);
	render_border(rect, border_width);

	deflate(rect, border_width);
	render_set_draw_color(
		button.is_pressed ? color_button_pressed : (
			button.is_highlighted ? color_button_highlighted : color_button_background));
	render_fill_rect(rect);

	deflate(rect, button_padding_ - extra_border_width);
	button_render_text(button, rect);
}

void Syscon::Impl::button_update_input_state(Button& button)
{
	button.is_highlighted = false;
	button.is_pressed = false;

	if (dragging_scrollbar_ != NULL)
	{
		return;
	}

	if (has_mouse_focus_ && SDL_PointInRect(&mouse_position_, &button.rect))
	{
		button.is_highlighted = true;

		if (input_state_.is_primary_button_pressed)
		{
			button.is_pressed = true;
			focused_control_ = &button;
		}

		if (old_input_state_.is_primary_button_pressed && !input_state_.is_primary_button_pressed)
		{
			if (button.clicked_callback != NULL)
			{
				(this->*button.clicked_callback)();
			}
		}
	}

	if (has_keyboard_focus_ && focused_control_ == &button)
	{
		const bool is_space_pressed = input_state_.pressed_keys[keyindex_space];

		const bool is_space_clicked =
			old_input_state_.pressed_keys[keyindex_space] &&
			!input_state_.pressed_keys[keyindex_space];

		if (is_space_pressed)
		{
			button.is_pressed = true;
			focused_control_ = &button;
		}

		if (is_space_clicked && button.clicked_callback != NULL)
		{
			(this->*button.clicked_callback)();
		}
	}
}

bool Syscon::Impl::edit_box_initialize(EditBox& edit_box)
{
	edit_box.is_highlighted = false;
	edit_box.cursor_glyph_index = 0;
	edit_box.window_pixel_offset = 0;
	return true;
}

void Syscon::Impl::edit_box_update_window_offset(EditBox& edit_box)
{
	const int cursor_pixel_offset = edit_box.cursor_glyph_index * (font_glyph_width_ + 1);

	if (cursor_pixel_offset < edit_box.window_pixel_offset ||
		cursor_pixel_offset >= edit_box.window_pixel_offset + edit_box.text_rect.w)
	{
		edit_box.window_pixel_offset = std::max(cursor_pixel_offset - edit_box_cursor_peek_length * (font_glyph_width_ + 1), 0);
	}
}

void Syscon::Impl::edit_box_update_cursor_by_mouse(EditBox& edit_box)
{
	if (mouse_position_.x < 0)
	{
		return;
	}

	const int pixel_offset = edit_box.window_pixel_offset + mouse_position_.x - edit_box.text_rect.x;
	edit_box.cursor_glyph_index = clamp(pixel_offset / (font_glyph_width_ + 1), 0, edit_box.glyph_indices.get_size());
	edit_box_update_window_offset(edit_box);
}

void Syscon::Impl::edit_box_reset_cursor_blinking()
{
	edit_box_cursor_is_visible_ = true;
	edit_box_cursor_last_ticks_ = SDL_GetTicks();
}

void Syscon::Impl::edit_box_update_cursor_blinking(EditBox& edit_box)
{
	if (focused_control_ != &edit_box)
	{
		return;
	}

	const Uint32 ticks = SDL_GetTicks();

	if (subtract_ticks(ticks, edit_box_cursor_last_ticks_) < edit_box_cursor_blinking_ticks)
	{
		return;
	}

	edit_box_cursor_is_visible_ = !edit_box_cursor_is_visible_;
	edit_box_cursor_last_ticks_ = ticks;
}

void Syscon::Impl::edit_box_clear(EditBox& edit_box)
{
	edit_box.text.clear();
	edit_box.text.get_data()[0] = '\0';

	edit_box.glyph_indices.clear();
	edit_box.cursor_glyph_index = 0;
	edit_box.window_pixel_offset = 0;
	edit_box_reset_cursor_blinking();
}

void Syscon::Impl::edit_box_render_text(EditBox& edit_box)
{
	if (edit_box.glyph_indices.is_empty() ||
		edit_box.text_rect.w <= 0 || edit_box.text_rect.h <= 0)
	{
		return;
	}

	const VectorU8& glyph_indices = edit_box.glyph_indices;
	const SDL_Rect& clip_rect = edit_box.text_rect;

	const int glyph_placement_width = font_glyph_width_ + 1;
	const int target_x_min = clip_rect.x - edit_box.window_pixel_offset % glyph_placement_width;
	const int target_x_max = clip_rect.x + clip_rect.w;

	SDL_Rect glyph_atlas_rect = SDL_Rect();
	glyph_atlas_rect.w = font_glyph_width;
	glyph_atlas_rect.h = font_glyph_height;

	SDL_Rect glyph_target_rect = SDL_Rect();
	glyph_target_rect.x = target_x_min;
	glyph_target_rect.y = clip_rect.y;
	glyph_target_rect.w = font_glyph_width_;
	glyph_target_rect.h = font_glyph_height_;

	render_set_clip_rect(edit_box.text_rect);
	render_set_font_color(color_edit_box_text);
	render_enable_font_blending();

	for (int i_glyph_index = edit_box.window_pixel_offset / glyph_placement_width, n_glyph_index = glyph_indices.get_size();
		i_glyph_index < n_glyph_index;
		++i_glyph_index)
	{
		if (glyph_target_rect.x >= target_x_max)
		{
			break;
		}

		const unsigned char glyph_index = glyph_indices[i_glyph_index];
		const FontGlyph& glyph = font_glyphs_[glyph_index];
		glyph_atlas_rect.x = glyph.atlas_x;
		glyph_atlas_rect.y = glyph.atlas_y;

		render_copy_font_glyph(glyph_atlas_rect, glyph_target_rect);

		glyph_target_rect.x += glyph_placement_width;
	}

	render_disable_font_blending();
}

void Syscon::Impl::edit_box_render_text_cursor(EditBox& edit_box)
{
	if (focused_control_ != &edit_box || SDL_RectEmpty(&edit_box.text_rect) || !edit_box_cursor_is_visible_)
	{
		return;
	}

	const int glyph_placement_width = font_glyph_width_ + 1;
	const int cursor_pixel_offset = edit_box.cursor_glyph_index * glyph_placement_width - edit_box.window_pixel_offset;

	const bool is_focused = focused_control_ == &edit_box;
	const int v_padding = default_border_width + is_focused * default_extra_border_width + 1;

	SDL_Rect clip_rect = SDL_Rect();
	clip_rect.x = edit_box.text_rect.x;
	clip_rect.y = edit_box.rect.y + v_padding;
	clip_rect.w = edit_box.text_rect.w;
	clip_rect.h = edit_box.rect.h - 2 * v_padding;

	if (SDL_RectEmpty(&clip_rect))
	{
		return;
	}

	render_set_clip_rect(clip_rect);
	render_set_draw_color(color_edit_box_text);

	const int line_x = edit_box.text_rect.x + cursor_pixel_offset;
	const int line_y1 = edit_box.rect.y + v_padding;
	const int line_y2 = edit_box.rect.y + edit_box.rect.h - default_border_width - is_focused * default_extra_border_width - 1;

	render_line(line_x, line_y1, line_x, line_y2);
}

void Syscon::Impl::edit_box_render(EditBox& edit_box)
{
	if (edit_box.rect.w <= 0 || edit_box.rect.h <= 0)
	{
		return;
	}

	const bool is_focused = focused_control_ == &edit_box;
	const int border_width = default_border_width + is_focused * default_extra_border_width;

	SDL_Rect rect = edit_box.rect;

	render_set_clip_rect(rect);
	render_set_draw_color(is_focused ? color_edit_box_border_focused : color_edit_box_border);
	render_border(rect, border_width);

	deflate(rect, border_width);
	render_set_draw_color(color_edit_box_background);
	render_fill_rect(rect);

	edit_box_render_text(edit_box);
	edit_box_render_text_cursor(edit_box);
}

void Syscon::Impl::edit_box_do_left(EditBox& edit_box)
{
	edit_box_reset_cursor_blinking();

	if (edit_box.cursor_glyph_index == 0)
	{
		return;
	}

	--edit_box.cursor_glyph_index;
	edit_box_update_window_offset(edit_box);
}

void Syscon::Impl::edit_box_do_right(EditBox& edit_box)
{
	edit_box_reset_cursor_blinking();
	const int glyph_count = edit_box.glyph_indices.get_size();

	if (edit_box.cursor_glyph_index == glyph_count)
	{
		return;
	}

	++edit_box.cursor_glyph_index;
	edit_box_update_window_offset(edit_box);
}

void Syscon::Impl::edit_box_do_home(EditBox& edit_box)
{
	edit_box_reset_cursor_blinking();
	edit_box.cursor_glyph_index = 0;
	edit_box.window_pixel_offset = 0;
}

void Syscon::Impl::edit_box_do_end(EditBox& edit_box)
{
	edit_box_reset_cursor_blinking();
	const int glyph_count = edit_box.glyph_indices.get_size();
	edit_box.cursor_glyph_index = glyph_count;
	const int total_width_pix = glyph_count * (font_glyph_width_ + 1) + 1;

	if (total_width_pix <= edit_box.text_rect.w)
	{
		edit_box.window_pixel_offset = 0;
	}
	else
	{
		edit_box.window_pixel_offset = total_width_pix - edit_box.text_rect.w;
	}
}

void Syscon::Impl::edit_box_insert_text(EditBox& edit_box)
{
	const int input_text_glyph_count = input_state_.glyph_indices.get_size();

	if (input_text_glyph_count == 0)
	{
		return;
	}

	const int insert_count = std::min(
		edit_box.glyph_indices.get_capacity() - edit_box.glyph_indices.get_size(),
		input_text_glyph_count);

	if (insert_count <= 0)
	{
		return;
	}

	edit_box.glyph_indices.insert_range(
		edit_box.cursor_glyph_index,
		input_state_.glyph_indices.get_data(),
		input_state_.glyph_indices.get_size());

	for (int i = 0; i < insert_count; ++i)
	{
		edit_box_do_right(edit_box);
	}

	edit_box_reset_cursor_blinking();
}

void Syscon::Impl::edit_box_do_del(EditBox& edit_box)
{
	edit_box_reset_cursor_blinking();

	if (edit_box.cursor_glyph_index >= edit_box.glyph_indices.get_size())
	{
		return;
	}

	edit_box.glyph_indices.erase(edit_box.cursor_glyph_index, 1);
}

void Syscon::Impl::edit_box_do_backspace(EditBox& edit_box)
{
	edit_box_reset_cursor_blinking();

	if (edit_box.cursor_glyph_index <= 0)
	{
		return;
	}

	edit_box_do_left(edit_box);
	edit_box_do_del(edit_box);
}

void Syscon::Impl::edit_box_update_input_state(EditBox& edit_box)
{
	edit_box.is_highlighted = false;

	if (dragging_scrollbar_ != NULL)
	{
		return;
	}

	if (has_mouse_focus_ && SDL_PointInRect(&mouse_position_, &edit_box.rect))
	{
		edit_box.is_highlighted = true;

		if (input_state_.is_primary_button_pressed)
		{
			focused_control_ = &edit_box;
			edit_box_reset_cursor_blinking();
			edit_box_update_cursor_by_mouse(edit_box);
		}
	}

	if (has_keyboard_focus_ && focused_control_ == &edit_box && !input_state_.is_primary_button_pressed)
	{
		edit_box_insert_text(edit_box);

		if (input_state_.repeated_keys[keyindex_enter])
		{
			if (edit_box.text_callback != NULL)
			{
				font_glyph_indices_to_zstring_clamped(edit_box.glyph_indices, edit_box.text);
				(this->*edit_box.text_callback)(edit_box.text.get_data());
			}
		}
		else if (input_state_.repeated_keys[keyindex_left])
		{
			edit_box_do_left(edit_box);
		}
		else if (input_state_.repeated_keys[keyindex_right])
		{
			edit_box_do_right(edit_box);
		}
		else if (input_state_.repeated_keys[keyindex_home])
		{
			edit_box_do_home(edit_box);
		}
		else if (input_state_.repeated_keys[keyindex_end])
		{
			edit_box_do_end(edit_box);
		}
		else if (input_state_.repeated_keys[keyindex_delete])
		{
			edit_box_do_del(edit_box);
		}
		else if (input_state_.repeated_keys[keyindex_backspace])
		{
			edit_box_do_backspace(edit_box);
		}
	}
}

bool Syscon::Impl::ui_copy_button_initialize()
{
	Button& button = ui_copy_button_;

	if (!button_initialize(button))
	{
		return false;
	}

	string_to_font_glyph_indices_clamped("Copy", button.glyph_indices);
	button.clicked_callback = &Impl::ui_copy_button_clicked_callback;
	return true;
}

void Syscon::Impl::ui_copy_button_lay_out()
{
	Button& button = ui_copy_button_;
	button_lay_out(button);
	button.rect.x = margin_;
}

void Syscon::Impl::ui_copy_button_render()
{
	button_render(ui_copy_button_);
}

void Syscon::Impl::ui_copy_button_update_input_state()
{
	button_update_input_state(ui_copy_button_);
}

void Syscon::Impl::ui_copy_button_clicked_callback()
{
	SDL_SetClipboardText(ui_log_.text.get_data());
}

bool Syscon::Impl::ui_clear_button_initialize()
{
	Button& button = ui_clear_button_;

	if (!button_initialize(button))
	{
		return false;
	}

	string_to_font_glyph_indices_clamped("Clear", button.glyph_indices);
	button.clicked_callback = &Impl::ui_clear_button_clicked_callback;
	return true;
}

void Syscon::Impl::ui_clear_button_lay_out()
{
	Button& button = ui_clear_button_;
	button_lay_out(button);
	button.rect.x = ui_copy_button_.rect.x + ui_copy_button_.rect.w + 2 * margin_;
}

void Syscon::Impl::ui_clear_button_render()
{
	button_render(ui_clear_button_);
}

void Syscon::Impl::ui_clear_button_update_input_state()
{
	button_update_input_state(ui_clear_button_);
}

void Syscon::Impl::ui_clear_button_clicked_callback()
{
	text_box_clear_text(ui_log_);
}

bool Syscon::Impl::ui_quit_button_initialize()
{
	Button& button = ui_quit_button_;

	if (!button_initialize(button))
	{
		return false;
	}

	string_to_font_glyph_indices_clamped("Quit", button.glyph_indices);
	button.clicked_callback = &Impl::ui_quit_button_clicked_callback;
	return true;
}

void Syscon::Impl::ui_quit_button_lay_out()
{
	Button& button = ui_quit_button_;
	button_lay_out(button);
	button.rect.x = window_width_ - margin_ - button_width_;
}

void Syscon::Impl::ui_quit_button_render()
{
	button_render(ui_quit_button_);
}

void Syscon::Impl::ui_quit_button_update_input_state()
{
	button_update_input_state(ui_quit_button_);
}

void Syscon::Impl::ui_quit_button_clicked_callback()
{
	SDL_Event sdl_event = SDL_Event();
	sdl_event.type = SDL_QUIT;
	SDL_PushEvent(&sdl_event);
}

bool Syscon::Impl::ui_input_line_initialize()
{
	EditBox& edit_box = ui_input_line_;
	edit_box_initialize(edit_box);

	if (!edit_box.text.reserve(ui_input_line_max_size))
	{
		return false;
	}

	if (!edit_box.glyph_indices.reserve(ui_input_line_max_length))
	{
		return false;
	}

	edit_box.text_callback = &Impl::ui_input_line_text_callback;
	return true;
}

void Syscon::Impl::ui_input_line_lay_out()
{
	if (is_error_mode_)
	{
		return;
	}

	const int height = font_glyph_height_ + 2 * (default_border_width + default_edit_box_padding);
	EditBox& edit_box = ui_input_line_;

	SDL_Rect rect = SDL_Rect();

	rect.x = margin_;
	rect.y = window_height_ - margin_ - button_height_ - margin_ - height;
	rect.w = window_width_ - 2 * margin_;
	rect.h = height;
	edit_box.rect = rect;

	deflate(rect, default_border_width + default_edit_box_padding);
	edit_box.text_rect = rect;

	edit_box_update_window_offset(edit_box);
}

void Syscon::Impl::ui_input_line_render()
{
	if (is_error_mode_)
	{
		return;
	}

	edit_box_render(ui_input_line_);
}

void Syscon::Impl::ui_input_line_text_callback(const char* text)
{
	if (callback_ != NULL)
	{
		const CallbackParam param(text);
		callback_(param);
	}

	edit_box_clear(ui_input_line_);
}

void Syscon::Impl::ui_input_line_update_input_state()
{
	EditBox& edit_box = ui_input_line_;
	edit_box_update_input_state(edit_box);
}

bool Syscon::Impl::scrollbar_initialize(Scrollbar& scrollbar)
{
	scrollbar = Scrollbar();
	scrollbar.is_visible = true;
	scrollbar.rect.w = default_scrollbar_width;
	return true;
}

void Syscon::Impl::scrollbar_update(Scrollbar& scrollbar)
{
	if (!scrollbar.is_visible)
	{
		return;
	}

	scrollbar.has_thumb = false;
	scrollbar.has_buttons = false;

	if (scrollbar.value < scrollbar.min_value || scrollbar.value > scrollbar.max_value)
	{
		return;
	}

	const int max_value_delta = scrollbar.max_value - scrollbar.min_value + scrollbar.page_size;

	if (max_value_delta <= scrollbar.page_size)
	{
		return;
	}

	const int shaft_length = scrollbar.rect.h - 2 * scrollbar_button_height;

	if (shaft_length >= 0)
	{
		scrollbar.has_buttons = true;

		scrollbar.upper_button_rect = scrollbar.rect;
		scrollbar.upper_button_rect.h = scrollbar_button_height;

		scrollbar.lower_button_rect = scrollbar.rect;
		scrollbar.lower_button_rect.y = scrollbar.rect.y + scrollbar.rect.h - scrollbar_button_height;
		scrollbar.lower_button_rect.h = scrollbar_button_height;
	}

	if (shaft_length <= scrollbar_min_thumb_height)
	{
		return;
	}

	const int thumb_height = std::max((scrollbar.page_size * shaft_length) / max_value_delta, scrollbar_min_thumb_height);
	const int thumb_offset = (scrollbar.value * shaft_length + max_value_delta - 1) / max_value_delta;

	scrollbar.has_thumb = true;

	scrollbar.thumb_rect.x = scrollbar.rect.x;
	scrollbar.thumb_rect.y = scrollbar.rect.y + scrollbar_button_height + thumb_offset;
	scrollbar.thumb_rect.w = scrollbar.rect.w;
	scrollbar.thumb_rect.h = thumb_height;
}

void Syscon::Impl::scrollbar_render_arrow(const SDL_Rect& button_rect, ScrollbarArrowDirection direction)
{
	const bool is_up = direction == scrollbar_arrow_direction_up;
	int target_x = button_rect.x + (button_rect.w - scrollbar_arrow_width) / 2;
	int target_y = button_rect.y + (button_rect.h - scrollbar_arrow_height) / 2;
	int bitmap_index = is_up ? 0 : scrollbar_arrow_height - 1;
	const int bitmap_step = is_up ? 1 : -1;

	for (int i_y = 0; i_y < scrollbar_arrow_height; ++i_y)
	{
		const unsigned short bitmap = scrollbar_up_arrow_bitmap[bitmap_index];

		for (int i_x = 0; i_x < scrollbar_arrow_width; ++i_x)
		{
			const int bit_index = scrollbar_arrow_width - 1 - i_x;
			const bool bit = (bitmap & (1 << bit_index)) != 0;

			if (bit)
			{
				render_point(target_x + i_x, target_y + i_y);
			}
		}

		bitmap_index += bitmap_step;
	}
}

void Syscon::Impl::scrollbar_render(const Scrollbar& scrollbar)
{
	if (!scrollbar.is_visible)
	{
		return;
	}

	render_set_clip_rect(scrollbar.rect);
	render_set_draw_color(color_scrollbar_track);
	render_fill_rect(scrollbar.rect);

	if (scrollbar.has_thumb)
	{
		render_set_draw_color(
			scrollbar.is_thumb_pressed ? color_scrollbar_thumb_pressed :
				(scrollbar.is_thumb_highlighted ? color_scrollbar_thumb_highlighted : color_scrollbar_thumb));

		render_fill_rect(scrollbar.thumb_rect);
	}

	if (scrollbar.has_buttons)
	{
		// Upper.
		//
		render_set_draw_color(
			scrollbar.is_upper_button_pressed ? color_scrollbar_button_pressed :
				(scrollbar.is_upper_button_highlighted ? color_scrollbar_button_highlighted : color_scrollbar_button));

		render_fill_rect(scrollbar.upper_button_rect);

		render_set_draw_color(
			scrollbar.is_upper_button_pressed ? color_scrollbar_arrow_pressed :
				(scrollbar.is_upper_button_highlighted ? color_scrollbar_arrow_highlighted : color_scrollbar_arrow));

		scrollbar_render_arrow(scrollbar.upper_button_rect, scrollbar_arrow_direction_up);

		// Lower.
		//
		render_set_draw_color(
			scrollbar.is_lower_button_pressed ? color_scrollbar_button_pressed :
				(scrollbar.is_lower_button_highlighted ? color_scrollbar_button_highlighted : color_scrollbar_button));

		render_fill_rect(scrollbar.lower_button_rect);

		render_set_draw_color(
			scrollbar.is_lower_button_pressed ? color_scrollbar_arrow_pressed :
				(scrollbar.is_lower_button_highlighted ? color_scrollbar_arrow_highlighted : color_scrollbar_arrow));

		scrollbar_render_arrow(scrollbar.lower_button_rect, scrollbar_arrow_direction_down);
	}
}

bool Syscon::Impl::text_box_initialize(TextBox& text_box, int text_capacity)
{
	if (!text_box.text.reserve(text_capacity) ||
		!text_box.glyph_indices.reserve(text_capacity) ||
		!text_box.line_offsets.reserve(text_capacity))
	{
		return false;
	}

	text_box.is_highlighted = false;
	text_box.border_color = color_text_box_border;
	text_box.highlighted_border_color = color_text_box_border_highlighted;
	text_box.focused_border_color = color_text_box_border_focused;
	text_box.background_color = color_text_box_background;
	text_box.text_color = color_text_box_text;

	return scrollbar_initialize(text_box.scrollbar);
}

void Syscon::Impl::text_box_update_scrollbar(TextBox& text_box)
{
	Scrollbar& scrollbar = text_box.scrollbar;

	scrollbar.line_size = font_glyph_height_;
	scrollbar.page_size = text_box.text_rect.h;

	scrollbar.min_value = 0;
	scrollbar.max_value = text_box.line_offsets.get_size() * font_glyph_height_ - scrollbar.page_size;

	scrollbar.value = scrollbar.max_value;

	if (scrollbar.min_value >= scrollbar.max_value || text_box.line_offsets.is_empty())
	{
		scrollbar.value = 0;
		scrollbar.min_value = 0;
		scrollbar.max_value = 0;
		scrollbar.line_size = 0;
		scrollbar.page_size = 0;
	}

	scrollbar_update(scrollbar);
}

void Syscon::Impl::text_box_lay_out(TextBox& text_box, SDL_Rect rect)
{
	Scrollbar& scrollbar = text_box.scrollbar;

	text_box.rect = rect;

	deflate(rect, default_border_width + default_extra_border_width);
	text_box.background_rect = rect;

	deflate(rect, default_text_box_padding);

	if (scrollbar.is_visible)
	{
		rect.w = std::max(rect.w - scrollbar.rect.w - default_text_box_padding, 0);
	}

	text_box.text_rect = rect;

	if (scrollbar.is_visible)
	{
		scrollbar.rect.x = rect.x + rect.w + default_text_box_padding;
		scrollbar.rect.y = rect.y;
		scrollbar.rect.h = rect.h;
	}
	else
	{
		scrollbar.rect.h = 0;
	}

	text_box_markup(text_box);
	text_box_update_scrollbar(text_box);
}

void Syscon::Impl::text_box_clear_text(TextBox& text_box)
{
	text_box.text.clear();
	text_box.glyph_indices.clear();
	text_box.line_offsets.clear();

	text_box_update_scrollbar(text_box);
}

void Syscon::Impl::text_box_markup(TextBox& text_box)
{
	assert(tab_size > 0);
	assert(text_box.text_rect.w >= 0);

	const int text_width = text_box.text_rect.w;
	const int char_count = text_box.text.get_size();

	if (char_count == 0 || text_width <= 0)
	{
		text_box.glyph_indices.clear();
		text_box.line_offsets.clear();
		return;
	}

	const int text_capacity = text_box.text.get_capacity();
	text_box.glyph_indices.resize_uninitialized(text_capacity);
	text_box.line_offsets.resize_uninitialized(text_capacity);

	const char* const chars = text_box.text.get_data();
	unsigned char* const glyph_indices = text_box.glyph_indices.get_data();
	unsigned short* const line_offsets = text_box.line_offsets.get_data();
	const int max_glyphs_per_line = std::max((text_width + font_glyph_width_ - 1) / font_glyph_width_, 1);
	int glyph_count = 0;
	int glyph_counter = 0;
	int extra_glyph_counter = 0;
	int line_count = 0;
	bool is_new_line = false;
	bool is_tab = false;
	int tab_offset = 0;
	int next_tab_offset = 0;
	line_offsets[line_count++] = 0;

	for (int i_char = 0; i_char < char_count; )
	{
		if (glyph_counter + extra_glyph_counter == max_glyphs_per_line)
		{
			is_new_line = true;
			extra_glyph_counter = 0;
		}

		if (is_new_line)
		{
			is_tab = false;
			is_new_line = false;
			glyph_counter = 0;
			extra_glyph_counter = 0;
			line_offsets[line_count++] = static_cast<unsigned short>(glyph_count);
			continue;
		}

		if (is_tab)
		{
			if (tab_offset == next_tab_offset)
			{
				is_tab = false;
			}
			else
			{
				++tab_offset;
				++extra_glyph_counter;
			}

			continue;
		}

		const int ch = chars[i_char];

		if (ch == '\t')
		{
			is_tab = true;
			next_tab_offset = tab_size * ((i_char / tab_size) + 1);
			tab_offset = ++i_char;
			glyph_indices[glyph_count++] = '\t';
			++glyph_counter;
			continue;
		}

		const bool is_last_char = i_char == char_count - 1;
		const int next_ch = is_last_char ? -1 : chars[i_char + 1];

		if (ch == '\r' || ch == '\n')
		{
			i_char += 1 + (ch == '\r' && next_ch == '\n');
			glyph_indices[glyph_count++] = '\n';
			is_new_line = true;
			continue;
		}

		int code_point = -1;
		const int decoded_char_count = utf8_decode(&chars[i_char], char_count - i_char, code_point);

		if (decoded_char_count <= 0)
		{
			break;
		}

		const int glyph_index = choose_font_glyph_index_for_code_point(code_point);
		glyph_indices[glyph_count++] = static_cast<unsigned char>(glyph_index);
		++glyph_counter;
		++i_char;
	}

	text_box.glyph_indices.resize(glyph_count);
	text_box.line_offsets.resize(line_count);
}

void Syscon::Impl::text_box_append_text(TextBox& text_box, const char* text)
{
	const int incoming_length = static_cast<int>(strlen(text));

	if (incoming_length == 0)
	{
		return;
	}

	const int max_length = text_box.text.get_capacity() - 1;
	const int current_length = text_box.text.get_size();
	const int combined_length = current_length + incoming_length;

	int old_move_src_offset = 0;
	int old_move_count = 0;
	int new_copy_src_offset = 0;
	int new_copy_dst_offset = 0;
	int new_copy_count = 0;

	if (combined_length <= max_length)
	{
		// Just append whole string.

		old_move_count = current_length;
		new_copy_dst_offset = old_move_count;
		new_copy_count = incoming_length;
	}
	else
	{
		if (incoming_length >= max_length)
		{
			// Push out all data.

			new_copy_count = max_length;
			new_copy_src_offset = incoming_length - max_length;
		}
		else
		{
			// Push out data partially.

			old_move_src_offset = combined_length - max_length;
			old_move_count = current_length - old_move_src_offset;
			new_copy_count = incoming_length;
			new_copy_dst_offset = max_length - incoming_length;
		}
	}

	char* chars = text_box.text.get_data();

	for (int i = 0; i < old_move_count; ++i)
	{
		chars[i] = chars[old_move_src_offset + i];
	}

	for (int i = 0; i < new_copy_count; ++i)
	{
		chars[new_copy_dst_offset + i] = text[new_copy_src_offset + i];
	}

	const int total_length = old_move_count + new_copy_count;
	chars[total_length] = '\0';
	text_box.text.resize_uninitialized(total_length);

	text_box_markup(text_box);
	text_box_update_scrollbar(text_box);
}

void Syscon::Impl::text_box_render_text(
	const TextBox& text_box,
	int line_index,
	int top_offset)
{
	if (line_index < 0 || line_index >= text_box.line_offsets.get_size() ||
		text_box.text_rect.w <= 0 || text_box.text_rect.h <= 0 ||
		text_box.glyph_indices.is_empty())
	{
		return;
	}

	const VectorU8& glyph_indices = text_box.glyph_indices;
	const int glyph_count = glyph_indices.get_size();

	const int text_width = std::max((text_box.text_rect.w / font_glyph_width_) * font_glyph_width_, font_glyph_width_);
	const int target_x_min = text_box.text_rect.x;
	const int target_x_max = text_box.text_rect.x + text_width;
	const int target_y_max = text_box.text_rect.y + text_box.text_rect.h;

	SDL_Rect glyph_atlas_rect = SDL_Rect();
	glyph_atlas_rect.w = font_glyph_width;
	glyph_atlas_rect.h = font_glyph_height;

	SDL_Rect glyph_target_rect = SDL_Rect();
	glyph_target_rect.y = text_box.text_rect.y + top_offset;
	glyph_target_rect.w = font_glyph_width_;
	glyph_target_rect.h = font_glyph_height_;

	render_set_clip_rect(text_box.text_rect);
	render_set_font_color(text_box.text_color);
	render_enable_font_blending();

	const int line_count = text_box.line_offsets.get_size();

	for (int i_line = line_index; i_line < line_count; ++i_line)
	{
		int tab_offset = 0;
		int i_glyph_index = text_box.line_offsets[i_line];
		glyph_target_rect.x = target_x_min;

		while (glyph_target_rect.x < target_x_max)
		{
			if (i_glyph_index == glyph_count)
			{
				break;
			}

			int glyph_index = glyph_indices[i_glyph_index];

			if (glyph_index == '\n')
			{
				break;
			}

			if (glyph_index == '\t')
			{
				const int next_tab_offset = (tab_offset / tab_size + 1) * tab_size;
				const int space_count = next_tab_offset - tab_offset;
				tab_offset += space_count;
				glyph_target_rect.x += space_count * font_glyph_width_;
			}
			else
			{
				const FontGlyph& glyph = font_glyphs_[glyph_index];
				glyph_atlas_rect.x = glyph.atlas_x;
				glyph_atlas_rect.y = glyph.atlas_y;

				render_copy_font_glyph(glyph_atlas_rect, glyph_target_rect);

				glyph_target_rect.x += font_glyph_width_;
				++tab_offset;
			}

			++i_glyph_index;
		}

		glyph_target_rect.y += font_glyph_height_;

		if (glyph_target_rect.y >= target_y_max)
		{
			break;
		}
	}

	render_disable_font_blending();
}

void Syscon::Impl::text_box_render(const TextBox& text_box)
{
	render_reset_clip_rect();

	const bool is_focused = const_cast<const void*>(focused_control_) == &text_box;

	render_set_draw_color(
		is_focused ? color_text_box_border_focused :
			(text_box.is_highlighted ? color_text_box_border_highlighted : color_text_box_border));

	render_border(text_box.rect, default_border_width + is_focused * default_extra_border_width);

	render_set_draw_color(text_box.background_color);
	render_fill_rect(text_box.background_rect);

	int text_line_index = 0;
	int text_top_offset = 0;

	if (text_box.scrollbar.is_visible)
	{
		text_top_offset = -text_box.scrollbar.value % font_glyph_height_;
		text_line_index = text_box.scrollbar.value / font_glyph_height_;
	}

	text_box_render_text(text_box, text_line_index, text_top_offset);
	scrollbar_render(text_box.scrollbar);
}

void Syscon::Impl::text_box_update_input_state(TextBox& text_box)
{
	text_box.is_highlighted = false;

	Scrollbar& scrollbar = text_box.scrollbar;
	scrollbar.is_thumb_highlighted = false;
	scrollbar.is_thumb_pressed = false;
	scrollbar.is_upper_button_highlighted = false;
	scrollbar.is_upper_button_pressed = false;
	scrollbar.is_lower_button_highlighted = false;
	scrollbar.is_lower_button_pressed = false;

	int scroll_delta = 0;
	const bool is_hit = SDL_PointInRect(&mouse_position_, &text_box.rect) == SDL_TRUE;

	if (has_mouse_focus_ && is_hit)
	{
		text_box.is_highlighted = true;

		if (input_state_.is_primary_button_pressed)
		{
			focused_control_ = &text_box;
		}

		if (scrollbar.is_visible)
		{
			if (SDL_PointInRect(&mouse_position_, &scrollbar.rect))
			{
				scrollbar.is_thumb_highlighted = scrollbar.has_thumb;
			}

			if (scrollbar.has_thumb)
			{
				if (SDL_PointInRect(&mouse_position_, &scrollbar.thumb_rect))
				{
					if (input_state_.is_primary_button_pressed)
					{
						scrollbar.is_thumb_pressed = true;

						if (!old_input_state_.is_primary_button_pressed)
						{
							dragging_scrollbar_ = &scrollbar;
							dragging_scrollbar_anchor_ = mouse_position_.y - scrollbar.thumb_rect.y;
						}
					}
				}
			}

			if (scrollbar.has_buttons && dragging_scrollbar_ == NULL)
			{
				if (SDL_PointInRect(&mouse_position_, &scrollbar.upper_button_rect))
				{
					scrollbar.is_upper_button_highlighted = true;

					if (input_state_.is_primary_button_pressed)
					{
						scrollbar.is_upper_button_pressed = true;
					}

					if (input_state_.is_primary_button_repeated)
					{
						scroll_delta += -scrollbar.line_size;
					}
				}

				if (SDL_PointInRect(&mouse_position_, &scrollbar.lower_button_rect))
				{
					scrollbar.is_lower_button_highlighted = true;

					if (input_state_.is_primary_button_pressed)
					{
						scrollbar.is_lower_button_pressed = true;
					}

					if (input_state_.is_primary_button_repeated)
					{
						scroll_delta += scrollbar.line_size;
					}
				}
			}
		}
	}

	if (has_keyboard_focus_ && focused_control_ == &text_box && dragging_scrollbar_ == NULL)
	{
		if (scrollbar.is_visible)
		{
			if (input_state_.repeated_keys[keyindex_down])
			{
				scroll_delta += scrollbar.line_size;
			}
			else if (input_state_.repeated_keys[keyindex_up])
			{
				scroll_delta += -scrollbar.line_size;
			}
			else if (input_state_.repeated_keys[keyindex_page_down])
			{
				scroll_delta += scrollbar.page_size;
			}
			else if (input_state_.repeated_keys[keyindex_page_up])
			{
				scroll_delta += -scrollbar.page_size;
			}
			else if (input_state_.pressed_keys[keyindex_lctrl] || input_state_.pressed_keys[keyindex_rctrl])
			{
				if (input_state_.repeated_keys[keyindex_home])
				{
					scroll_delta += -scrollbar.value;
				}
				else if (input_state_.repeated_keys[keyindex_end])
				{
					scroll_delta += scrollbar.max_value - scrollbar.value;
				}
			}
		}
	}

	if (is_hit && input_state_.mouse_wheel_v_delta != 0 && dragging_scrollbar_ == NULL)
	{
		scroll_delta += -input_state_.mouse_wheel_v_delta * scrollbar.line_size * mouse_wheel_lines_per_scroll;
	}

	scrollbar.value = clamp(scrollbar.value + scroll_delta, scrollbar.min_value, scrollbar.max_value);
	scrollbar_update(scrollbar);
}

bool Syscon::Impl::ui_log_initialize()
{
	TextBox& text_box = ui_log_;

	if (!text_box_initialize(text_box, ui_log_text_capacity))
	{
		return false;
	}

	text_box.scrollbar.is_visible = true;
	return true;
}

void Syscon::Impl::ui_log_lay_out()
{
	TextBox& text_box = ui_log_;
	SDL_Rect rect = SDL_Rect();
	rect.x = margin_;
	rect.y = (is_error_mode_ ? ui_error_label_.rect.y + ui_error_label_.rect.h : 0) + margin_;
	rect.w = window_width_ - 2 * margin_;
	rect.h = (is_error_mode_ ? ui_copy_button_.rect.y : ui_input_line_.rect.y) - rect.y - margin_;
	text_box_lay_out(text_box, rect);
}

void Syscon::Impl::ui_log_update_input_state()
{
	text_box_update_input_state(ui_log_);
}

void Syscon::Impl::exclude_input_line_from_focusables()
{
	void** const iter_begin = focusable_controls_.begin();
	void** const iter_end = focusable_controls_.end();
	void** const iter = std::remove(iter_begin, iter_end, static_cast<void*>(&ui_input_line_));

	if (iter == iter_end)
	{
		// Already removed.
		return;
	}

	focusable_controls_count_ = static_cast<int>(iter - iter_begin);
	focused_control_ = &ui_quit_button_;
}

void Syscon::Impl::lay_out_controls()
{
	ui_error_label_lay_out();
	ui_copy_button_lay_out();
	ui_clear_button_lay_out();
	ui_quit_button_lay_out();
	ui_input_line_lay_out();
	ui_log_lay_out();
}

int Syscon::Impl::find_focused_control_index() const
{
	if (focused_control_ == NULL)
	{
		return -1;
	}

	for (int i = 0; i < focusable_controls_count_; ++i)
	{
		if (focusable_controls_[i] == focused_control_)
		{
			return i;
		}
	}

	return -1;
}

void Syscon::Impl::update_controls_dragging_scrollbar()
{
	if (!has_mouse_focus_ ||
		(!input_state_.is_primary_button_pressed &&
			old_input_state_.is_primary_button_pressed))
	{
		dragging_scrollbar_ = NULL;
		return;
	}

	if (dragging_scrollbar_ == NULL || !has_mouse_focus_)
	{
		return;
	}

	Scrollbar& scrollbar = *dragging_scrollbar_;
	scrollbar.is_thumb_pressed = true;

	const int value_max_delta = scrollbar.max_value - scrollbar.min_value;

	if (mouse_position_.y < 0 || value_max_delta <= 0)
	{
		return;
	}

	if (dragging_scrollbar_anchor_ >= scrollbar.thumb_rect.h)
	{
		dragging_scrollbar_anchor_ = scrollbar.thumb_rect.h / 2;
	}

	const int shaft_y_min = scrollbar.upper_button_rect.y + scrollbar.upper_button_rect.h;
	const int shaft_y_max = scrollbar.lower_button_rect.y;
	const int shaft_length = shaft_y_max - shaft_y_min - scrollbar.thumb_rect.h;

	const int new_thumb_y = clamp(
		mouse_position_.y - dragging_scrollbar_anchor_,
		shaft_y_min,
		shaft_y_max - scrollbar.thumb_rect.h);

	const int offset = new_thumb_y - shaft_y_min;
	const int new_value = (offset * value_max_delta) / shaft_length;
	scrollbar.value = new_value;
	scrollbar_update(scrollbar);
}

void Syscon::Impl::update_primary_button_repeating()
{
	if (!has_keyboard_focus_ ||
		input_state_.is_primary_button_repeated ||
		!input_state_.is_primary_button_pressed)
	{
		return;
	}

	const Uint32 ticks = SDL_GetTicks();
	const Uint32 duration = subtract_ticks(ticks, input_state_.primary_button_last_repeat_ticks);

	if (duration < mouse_button_repeating_period_ticks)
	{
		return;
	}

	input_state_.primary_button_last_repeat_ticks = ticks;
	input_state_.is_primary_button_repeated = true;
}

void Syscon::Impl::update_controls_keyboard_navigation()
{
	if (!has_keyboard_focus_ || dragging_scrollbar_ != NULL)
	{
		return;
	}

	const bool is_lshift_pressed = input_state_.pressed_keys[keyindex_lshift];
	const bool is_rshift_pressed = input_state_.pressed_keys[keyindex_rshift];
	const bool is_shift_pressed = is_lshift_pressed || is_rshift_pressed;

	const bool is_tab_clicked =
		old_input_state_.pressed_keys[keyindex_tab] &&
		!input_state_.pressed_keys[keyindex_tab];

	if (is_tab_clicked)
	{
		int focused_control_index = find_focused_control_index();

		if (focused_control_index < 0)
		{
			if (focusable_controls_count_ > 0)
			{
				focused_control_ = focusable_controls_[0];
			}
			else
			{
				focused_control_ = NULL;
			}
		}
		else
		{
			focused_control_index += is_shift_pressed ? -1 : 1;

			if (focused_control_index < 0)
			{
				focused_control_index = focusable_controls_count_ - 1;
			}
			else if (focused_control_index == focusable_controls_count_)
			{
				focused_control_index = 0;
			}

			focused_control_ = focusable_controls_[focused_control_index];
		}
	}
}

void Syscon::Impl::update_controls_edit_box_blinking_cursor()
{
	edit_box_update_cursor_blinking(ui_input_line_);
}

void Syscon::Impl::update_controls_input_state()
{
	update_primary_button_repeating();
	update_controls_keyboard_navigation();
	update_controls_edit_box_blinking_cursor();

	ui_log_update_input_state();
	ui_copy_button_update_input_state();
	ui_clear_button_update_input_state();
	ui_quit_button_update_input_state();
	ui_input_line_update_input_state();

	update_controls_dragging_scrollbar();

	old_input_state_ = input_state_;
	input_state_.repeated_keys.fill(false);
	input_state_.is_primary_button_repeated = false;
	input_state_.mouse_wheel_v_delta = 0;
	input_state_.glyph_indices.clear();
}

void Syscon::Impl::render_controls()
{
	ui_error_label_render();
	ui_copy_button_render();
	ui_clear_button_render();
	ui_quit_button_render();
	ui_input_line_render();
	text_box_render(ui_log_);
}

// ==========================================================================

Syscon::Syscon()
{
	impl_ = static_cast<Impl*>(malloc(sizeof(Impl)));

	if (impl_ != NULL)
	{
		::new (static_cast<void*>(impl_)) Impl();
	}
}

Syscon::~Syscon()
{
	if (impl_ != NULL)
	{
		impl_->~Impl();
		free(impl_);
	}
}

bool Syscon::is_initialized() const
{
	return impl_ != NULL && impl_->is_initialized();
}

const char* Syscon::get_error_message() const
{
	if (!is_initialized())
	{
		return "Not initialized.";
	}

	return impl_->get_error_message();
}

const char* Syscon::get_text() const
{
	if (!is_initialized())
	{
		return "";
	}

	return impl_->get_text();
}

bool Syscon::initialize(Callback callback)
{
	if (impl_ == NULL)
	{
		return false;
	}

	if (!impl_->initialize(callback))
	{
		impl_->terminate();
		return false;
	}

	return true;
}

void Syscon::terminate()
{
	if (impl_ != NULL)
	{
		impl_->terminate();
	}
}

void Syscon::set_title(const char* title)
{
	if (is_initialized())
	{
		impl_->set_title(title);
	}
}

void Syscon::show(ShowMode mode)
{
	if (is_initialized())
	{
		impl_->show(mode);
	}
}

void Syscon::clear_text()
{
	if (is_initialized())
	{
		impl_->clear_text();
	}
}

void Syscon::append_text(const char* text)
{
	if (is_initialized())
	{
		impl_->append_text(text);
	}
}

void Syscon::set_error_text(const char* text)
{
	if (is_initialized())
	{
		impl_->set_error_text(text);
	}
}

void Syscon::update()
{
	if (is_initialized())
	{
		impl_->update();
	}
}

void Syscon::handle_event(const SDL_Event& sdl_event)
{
	if (is_initialized())
	{
		impl_->handle_event(sdl_event);
	}
}

void Syscon::run()
{
	if (is_initialized())
	{
		impl_->run();
	}
}

} // namespace rtcw
