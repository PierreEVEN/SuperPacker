
#include <windows.h>


#include "Logger.h"
#include "OpenGLContext.h"
#include "SuperPacker.h"

#if _WIN32
#define ARGV __argv
#define ARGC __argc
#else
#define ARGV argv
#define ARGC argc
#endif

#if _WIN32 && false
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	current_path(std::filesystem::path(ARGV[0]).parent_path());
	
	OpenGLContext::Init();
	
	auto packer = std::make_shared<SuperPacker::ImagePacker>("config/default.ini", ARGC > 1 ? ARGV[1] : std::optional<std::filesystem::path>()
	);

	packer->add_format({ "PNG file", "*.png" });
	packer->add_format({ "TGA file", "*.tga" });
	packer->add_format({ "JPEG file", "*.jpg|*.jpeg|*.JPEG|*.JPG" });
	packer->add_format({ "BITMAP file", "*.bmp" });
	packer->add_format({ "HDR file", "*.hdr" });
	packer->add_format({ "Any file", "*.*" });

	packer->add_channel({0, "red","r",{1.0, 0.0, 0.0, 1.0},0});
	packer->add_channel({1, "green","g",{0.0, 1.0, 0.0, 1.0},0});
	packer->add_channel({2, "blue","b",{0.0, 0.0, 1.0, 1.0},0});
	packer->add_channel({3, "alpha","a",{1.0, 1.0, 1.0, 1.0},255});

	packer->add_channel_combination({ "grayscale", {"r"} });
	packer->add_channel_combination({ "rgb", {"r", "g", "b"} });
	packer->add_channel_combination({ "rgba", {"r", "g", "b", "a"} });

	packer->set_current_channel_combination("rgba");
	packer->set_current_export_format("png");

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
