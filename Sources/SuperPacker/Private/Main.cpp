
#include "OpenGLContext.h"
#include "imgui.h"
#include "SuperPacker.h"

#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
//int main(int argc, char** argv)
{
	OpenGLContext::Init();
	

	
	while (!OpenGLContext::ShouldClose()) {		
		OpenGLContext::BeginFrame();
		
		int size_x, size_y;
		OpenGLContext::GetWindowSize(size_x, size_y);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(size_x), static_cast<float>(size_y)));
		
		if (ImGui::Begin("background_window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			SuperPacker::draw_ui();
		}
		ImGui::End();
		
		OpenGLContext::EndFrame();
	}

	OpenGLContext::Shutdown();

	return 0;
}
