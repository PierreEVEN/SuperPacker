#include "OpenGLContext.h"


#include <filesystem>
#include <iostream>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>


#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "Logger.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

GLFWwindow* WindowHandle = nullptr;
bool close = false;

GLFWwindow* OpenGLContext::get_window_handle()
{
	return WindowHandle;
}

void ResizeCallback(GLFWwindow* windows, int x, int y) {
	glViewport(0, 0, x, y);
}

void ErrorCallback(int Code, const char* Message) {
	logger_warning("GLFW error %d : %s", Code, Message);
}

void CheckGLErrors()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		logger_error("GL error : %s", err);
	}
}

void OpenGLContext::Init() {

	/***
	 * Init openGL
	 */
	logger_log("Create Glfw context");
	if (!glfwInit()) std::cerr << "Failed to create glfw window" << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

	WindowHandle = glfwCreateWindow(1000, 620, std::string("Super packer - OpenGL - Version " + std::string(VERSION)).c_str(), 0, 0);
	if (!WindowHandle) std::cerr << "Failed to create Glfw window handle" << std::endl;
	glfwMakeContextCurrent(WindowHandle);
	glfwSetFramebufferSizeCallback(WindowHandle, &ResizeCallback);
	glfwSetErrorCallback(&ErrorCallback);
	glfwSetInputMode(WindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	GLFWimage icons[1];
	int channels;
	icons[0].pixels = stbi_load("icon.png", &icons[0].width, &icons[0].height, &channels, 4);
	glfwSetWindowIcon(WindowHandle, 1, icons);
	stbi_image_free(icons[0].pixels);
	

	logger_log("Initialize OpenGL context");
	if (gl3wInit()) {
		logger_error("failed to initialize OpenGL");
	}
	if (!gl3wIsSupported(3, 2)) {
		logger_error("OpenGL 3.2 not supported");
	}
	logger_validate("OpenGL %s, GLSL : %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
	

	/***
	 * Init Imgui
	 */
	logger_log("Initialize ImGui ");
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.IniFilename = NULL;
	
	auto& style = ImGui::GetStyle();
	style.WindowBorderSize = 0;
	style.FramePadding = ImVec2(20, 10);

	if (std::filesystem::exists("Roboto-Medium.ttf")) {
		io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 20.f);
	}
	
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::StyleColorsDark();
			
	ImGui_ImplGlfw_InitForOpenGL(WindowHandle, true);
	ImGui_ImplOpenGL3_Init("#version 150");
}

bool OpenGLContext::ShouldClose() {
	return glfwWindowShouldClose(WindowHandle) || close;
}

void OpenGLContext::BeginFrame() {

	if (!WindowHandle) logger_error("null window");
	
	glfwPollEvents();	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void OpenGLContext::EndFrame()
{
	ImGui::Render();
	glClearColor(.2f, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(WindowHandle);
}

void OpenGLContext::Shutdown() {

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	glfwTerminate();
}

void OpenGLContext::GetWindowSize(int& SizeX, int& SizeY) {
	glfwGetWindowSize(WindowHandle, &SizeX, &SizeY);
}

void OpenGLContext::request_close()
{
	close = true;
}
