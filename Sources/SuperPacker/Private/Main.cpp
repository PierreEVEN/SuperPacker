
#include <windows.h>



#include "Logger.h"
#include "OpenGLContext.h"
#include "GL/gl3w.h"
#include <GLFW/glfw3.h>
#include "SuperPacker.h"

#if _WIN32
#define ARGV __argv
#define ARGC __argc
#else
#define ARGV argv
#define ARGC argc
#endif

std::shared_ptr<SuperPacker::ImagePacker> packer;

#if _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	current_path(std::filesystem::path(ARGV[0]).parent_path());
	
	OpenGLContext::Init();
	
	packer = std::make_shared<SuperPacker::ImagePacker>("config/default.ini");

	packer->add_format({ "PNG file", "*.png", "png" });
	packer->add_format({ "TGA file", "*.tga", "tga" });
	packer->add_format({ "JPEG file", "*.jpg;*.jpeg;*.JPEG;*.JPG", "jpg" });
	packer->add_format({ "BITMAP file", "*.bmp", "bmp" });

	packer->add_channel({0, "red","r",{1, 0.3f, 0.3f, 1},0});
	packer->add_channel({1, "green","g",{0.1f, 0.6f, 0.1f, 1},0});
	packer->add_channel({2, "blue","b",{0.5f, 0.5f, 1, 1},0});
	packer->add_channel({3, "alpha","a",{0.5f, 0.5f, 0.5f, 1},255});

	packer->add_channel_combination({ "grayscale", {"r"} });
	packer->add_channel_combination({ "rgb", {"r", "g", "b"} });
	packer->add_channel_combination({ "rgba", {"r", "g", "b", "a"} });

	if (ARGC > 1) packer->reset_from_source(ARGV[1]);


	glfwSetDropCallback(OpenGLContext::get_window_handle(), [](GLFWwindow* window, int count, const char** paths)
	{
			for (int i = 0; i < count; ++i) packer->drop_file(paths[i]);
	});
	
	while (!OpenGLContext::ShouldClose()) {		
		OpenGLContext::BeginFrame();
		
		int size_x, size_y;
		OpenGLContext::GetWindowSize(size_x, size_y);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(size_x), static_cast<float>(size_y)));
		
		if (ImGui::Begin("background_window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			packer->draw_ui();
		}
		ImGui::End();
		
		OpenGLContext::EndFrame();
	}

	OpenGLContext::Shutdown();

	return 0;
}
