
#include "OpenGLContext.h"
#include "imgui.h"
#include "SuperPacker.h"

#include <windows.h>
//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
int main(int argc, char** argv)
{
	OpenGLContext::Init();

	std::vector<std::pair<std::string, std::string>> formats = {
		{"PNG file", "*.png"},
		{"TGA file", "*.tga"},
		{"Jpeg file", "*.jpg"},
		{"Bitmap file", "*.bmp"},
		{"Any file", "*.*"},
	};
	
	std::vector<SuperPacker::ChannelInfo> channel_infos
	{
		{
			.channel_name = "R",
			.channel_color = ImVec4(1, 0.3f, 0.3f, 1),
			.default_value = 0
		},
		{
			.channel_name = "G",
			.channel_color = ImVec4(0.1f, 0.6f, 0.1f, 1),
			.default_value = 0
		},
		{
			.channel_name = "B",
			.channel_color = ImVec4(0.5f, 0.5f, 1, 1),
			.default_value = 0
		},
		{
			.channel_name = "A",
			.channel_color = ImVec4(0.5f, 0.5f, 0.5f, 1),
			.default_value = 255
		}
	};
	
	std::vector<SuperPacker::ChannelConfiguration> configurations = {
		SuperPacker::ChannelConfiguration {
			.configuration_name = "Grayscale",
			.channels = {
				{
					.channel_id = 0
				}
			},
		},
		SuperPacker::ChannelConfiguration {
			.configuration_name = "RGB",
			.channels = {
				{
					.channel_id = 0
				},
				{
					.channel_id = 1
				},
				{
					.channel_id = 2
				}
			},
		},
		SuperPacker::ChannelConfiguration {
			.configuration_name = "RGBA",
			.channels = {
				{
					.channel_id = 0
				},
				{
					.channel_id = 1
				},
				{
					.channel_id = 2
				},
				{
					.channel_id = 3
				}
			},
		},
	};
	
	auto packer = std::make_shared<SuperPacker::ImagePacker>(
		"config/default.ini",
		formats,
		channel_infos,
		configurations
	);
	
	
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
