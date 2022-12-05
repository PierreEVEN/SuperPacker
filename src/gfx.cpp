#include "gfx.h"

#include <FreeImagePlus.h>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <GLFW/glfw3.h>
#include <gl/gl3w.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <Windows.h>

#include "resource.h"


static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "Glfw Error " << error << ": " << description << std::endl;
}

#define COLOR_DARKNESS 400.f
#define COLOR(r, g, b, a) ImVec4(r / COLOR_DARKNESS, g / COLOR_DARKNESS, b / COLOR_DARKNESS, a / 255.f)
#define COLOR_BR(r, g, b, a) ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f)

Gfx::Gfx(const std::string& window_name, uint32_t window_x, uint32_t window_y)
{
	glfwSetErrorCallback(glfw_error_callback);
	{
		if (!glfwInit()) { std::cerr << "Failed to initialize GFLW!" << std::endl; }
	}
	const char* glsl_version = "#version 330";
	GLint major = 4, minor = 5;
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
#endif

	{
		main_window = glfwCreateWindow(window_x, window_y, window_name.c_str(), nullptr, nullptr);
		if (!main_window) { std::cerr << "Failed to create Window!" << std::endl; }
	}
	glfwMakeContextCurrent(main_window);

	bool err = gl3wInit() != 0;
	if (err) { std::cerr << "Failed to initialize OpenGL loader!" << std::endl; }

	glfwSwapInterval(1); // Enable vsync
	glfwSetWindowUserPointer(main_window, this);

	if (const HICON icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1)))
	{
		ICONINFO icon_info;
		if (GetIconInfo(icon, &icon_info))
		{
			BITMAP bitmap;
			if (GetObject(icon_info.hbmColor, sizeof(bitmap), &bitmap) && GetObject(
				icon_info.hbmMask, sizeof(bitmap), &bitmap))
			{
				const int width = bitmap.bmWidth;
				const int height = bitmap.bmHeight;

				const fipImage img(FIT_BITMAP, width, height, 32);

				if (GetBitmapBits(icon_info.hbmColor, img.getScanWidth() * height, img.accessPixels()))
				{
					const GLFWimage ico_image = {
						.width = static_cast<int>(img.getWidth()),
						.height = static_cast<int>(img.getHeight()),
						.pixels = img.accessPixels()
					};
					glfwSetWindowIcon(main_window, 1, &ico_image);
				}

				DeleteObject(icon_info.hbmColor);
				DeleteObject(icon_info.hbmMask);
			}
		}
		DestroyIcon(icon);
	}


	GL_CHECK_ERROR();
	IMGUI_CHECKVERSION();
	imgui_context
		=
		ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();


	if (const HRSRC icon_hrc = FindResourceA(GetModuleHandle(NULL), MAKEINTRESOURCE(IDF_ROBOTOMEDIUM), RT_FONT))
	{
		const DWORD font_data_size = SizeofResource(GetModuleHandle(NULL), icon_hrc);
		HGLOBAL font_data_handle = LoadResource(GetModuleHandle(NULL), icon_hrc);
		const LPVOID font_data = LockResource(font_data_handle);
		io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 16.f);
		UnlockResource(font_data_handle);
		FreeResource(font_data_handle);
	}

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io. BackendFlags
		|=
		ImGuiBackendFlags_HasMouseCursors;
	io
		.
		BackendFlags
		|=
		ImGuiBackendFlags_HasSetMousePos;

	auto& style = ImGui::GetStyle();
	style
		.
		WindowRounding
		=
		0;
	style
		.
		ScrollbarRounding
		=
		0;
	style
		.
		FramePadding
		=
		ImVec2(

			10
			,
			2
		);
	style
		.
		ItemSpacing
		=
		ImVec2(

			8
			,
			3
		);
	style
		.
		WindowTitleAlign
		=
		ImVec2(

			0.5f
			,
			0.5
		);
	style
		.
		WindowPadding
		=
		ImVec2(

			4
			,
			4
		);
	style
		.
		GrabMinSize
		=
		12;
	style
		.
		IndentSpacing
		=
		17;
	style
		.
		WindowBorderSize
		=
		0;
	style
		.
		ChildBorderSize
		=
		0;
	style
		.
		PopupBorderSize
		=
		0;
	style
		.
		TabRounding
		=
		0;
	style
		.
		ScrollbarSize
		=
		10;
	style
		.
		FrameBorderSize
		=
		1;
	style
		.
		PopupBorderSize
		=
		1;

	style
		.
		Colors[ImGuiCol_Text] = COLOR_BR(232, 232, 232, 255);
	style
		.
		Colors[ImGuiCol_FrameBg] = COLOR(47, 47, 47, 255);
	style
		.
		Colors[ImGuiCol_ChildBg] = COLOR(0, 0, 0, 39);
	style
		.
		Colors[ImGuiCol_FrameBgHovered] = COLOR(102, 102, 102, 255);
	style
		.
		Colors[ImGuiCol_FrameBgActive] = COLOR(85, 85, 85, 255);
	style
		.
		Colors[ImGuiCol_WindowBg] = COLOR(65, 65, 65, 255);
	style
		.
		Colors[ImGuiCol_Border] = COLOR(0, 0, 0, 255);
	style
		.
		Colors[ImGuiCol_Button] = COLOR(100, 100, 100, 255);
	style
		.
		Colors[ImGuiCol_ButtonHovered] = COLOR(140, 140, 140, 255);
	style
		.
		Colors[ImGuiCol_ButtonActive] = COLOR(78, 78, 78, 255);
	style
		.
		Colors[ImGuiCol_CheckMark] = COLOR(255, 222, 139, 255);
	style
		.
		Colors[ImGuiCol_SliderGrab] = COLOR(142, 142, 142, 255);
	style
		.
		Colors[ImGuiCol_SliderGrabActive] = COLOR(210, 210, 210, 255);
	style
		.
		Colors[ImGuiCol_Tab] = COLOR(0, 0, 0, 0);
	style
		.
		Colors[ImGuiCol_TabActive] = COLOR(86, 86, 86, 255);
	style
		.
		Colors[ImGuiCol_TabHovered] = COLOR(140, 140, 140, 255);
	style
		.
		Colors[ImGuiCol_TabUnfocusedActive] = COLOR(63, 63, 63, 255);
	style
		.
		Colors[ImGuiCol_TitleBgActive] = COLOR(48, 48, 48, 255);
	style
		.
		Colors[ImGuiCol_Header] = COLOR(73, 73, 73, 255);
	style
		.
		Colors[ImGuiCol_HeaderActive] = COLOR(48, 48, 48, 255);
	style
		.
		Colors[ImGuiCol_HeaderHovered] = COLOR(109, 109, 109, 255);
	style
		.
		Colors[ImGuiCol_TextSelectedBg] = COLOR(153, 153, 153, 255);
	style
		.
		Colors[ImGuiCol_NavHighlight] = COLOR(250, 203, 66, 255);
	style
		.
		Colors[ImGuiCol_PopupBg] = COLOR(70, 70, 70, 255);

#ifdef _WIN32
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	float xscale, yscale;
	glfwGetMonitorContentScale(monitor, &xscale, &yscale);
	if
	(xscale
		>
		1
		||
		yscale
		>
		1
	)
	{
		const float highDPIscaleFactor = 1;
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(highDPIscaleFactor);
		ImGui::GetIO().FontGlobalScale = highDPIscaleFactor;
	}
#endif
	{
		GL_CHECK_ERROR();
		ImGui_ImplGlfw_InitForOpenGL(main_window, true);

		GL_CHECK_ERROR();
		ImGui_ImplOpenGL3_Init(glsl_version);
		while (glGetError() != GL_NO_ERROR);
	}
	GL_CHECK_ERROR();
}

bool Gfx::draw()
{
	glfwPollEvents();

	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (glfwGetCurrentContext() != main_window)
		glfwMakeContextCurrent(main_window);

	on_draw.execute();

	ImGui::Render();
	GL_CHECK_ERROR();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	GL_CHECK_ERROR();
	glfwSwapBuffers(main_window);
	GL_CHECK_ERROR();
	return !glfwWindowShouldClose(main_window);
}
