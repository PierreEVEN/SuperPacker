#include "OpenGLContext.h"

#include <iostream>

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

GLFWwindow* WindowHandle = nullptr;

void ResizeCallback(GLFWwindow* windows, int x, int y) {
	glViewport(0, 0, x, y);
}

void ErrorCallback(int Code, const char* Message) {
	std::cerr << "GLFW error " << Code << " : " << Message << std::endl;
}

void CheckGLErrors()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "GL error : " << err << std::endl;
	}
}

void OpenGLContext::Init() {

	/***
	 * Init openGL
	 */
	std::cout << "Create Glfw context" << std::endl;
	if (!glfwInit()) std::cerr << "Failed to create glfw window" << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

	WindowHandle = glfwCreateWindow(1000, 1000, "Pure3D - OpenGL", 0, 0);
	if (!WindowHandle) std::cerr << "Failed to create Glfw window handle" << std::endl;
	glfwMakeContextCurrent(WindowHandle);
	glfwSetFramebufferSizeCallback(WindowHandle, &ResizeCallback);
	glfwSetErrorCallback(&ErrorCallback);
	glfwSetInputMode(WindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);


	std::cout << "Initialize OpenGL context" << std::endl;
	if (gl3wInit()) {
		std::cerr << "failed to initialize OpenGL" << std::endl;
	}
	if (!gl3wIsSupported(3, 2)) {
		std::cerr << "OpenGL 3.2 not supported" << std::endl;
	}
	std::cout << "OpenGL " << glGetString(GL_VERSION) << ", GLSL : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;



	/***
	 * Init Imgui
	 */
	std::cout << "Initialize ImGui " << std::endl;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::StyleColorsDark();
			
	ImGui_ImplGlfw_InitForOpenGL(WindowHandle, true);
	ImGui_ImplOpenGL3_Init("#version 150");


}

bool OpenGLContext::ShouldClose() {
	return glfwWindowShouldClose(WindowHandle);
}

void OpenGLContext::BeginFrame() {

	if (!WindowHandle) std::cerr << "null window" << std::endl;
	
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
