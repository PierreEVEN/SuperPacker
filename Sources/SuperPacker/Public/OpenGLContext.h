#pragma once

struct GLFWwindow;

namespace OpenGLContext
{

	void Init();

	bool ShouldClose();
	void BeginFrame();
	void EndFrame();
	void Shutdown();
	void GetWindowSize(int& SizeX, int& SizeY);
	void request_close();

	GLFWwindow* get_window_handle();
}