#pragma once
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <gl/gl3w.h>
#include "event_manager.h"

struct ImGuiContext;
struct GLFWwindow;

DECLARE_DELEGATE_MULTICAST(EventDraw);

#ifndef GL_TABLE_TOO_LARGE
#define GL_TABLE_TOO_LARGE 0x8031
#endif

inline void GL_CHECK_ERROR()
{
	int gl_error_id = glGetError();

	while (gl_error_id != GL_NO_ERROR)
	{
		std::cerr << "opengl error : ";
		switch (gl_error_id)
		{
		case GL_NO_ERROR:
			std::cerr << "GL_NO_ERROR";
			break;
		case GL_INVALID_ENUM:
			std::cerr << "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			std::cerr << "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << "GL_INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << "GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			std::cerr << "GL_STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << "GL_OUT_OF_MEMORY";
			break;
		case GL_TABLE_TOO_LARGE:
			std::cerr << "GL_TABLE_TOO_LARGE";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		default:
			std::cerr << "unknown error";
		}
		std::cerr << std::endl;

#ifdef _DEBUG
		assert((void("opengl error"), gl_error_id == GL_NO_ERROR));
#endif
		gl_error_id = glGetError();
	}
}

class Gfx
{
public:
	Gfx(const std::string& window_name, uint32_t window_x, uint32_t window_y, int* pos_x = nullptr, int* pos_y = nullptr);

	EventDraw on_draw;

	bool draw();

	static Gfx& get();

	float get_delta_second() const { return delta_second; }

	[[nodiscard]] std::string get_clipboard() const;
	void set_clipboard(const std::string& data) const;

	int get_window_width() const;
	int get_window_height() const;
	
	int get_window_pos_x() const;
	int get_window_pos_y() const;
private:
	float delta_second = 0;
	double last_time = 0;

	GLFWwindow* main_window;
	ImGuiContext* imgui_context;
};
