
#include "OpenGLContext.h"
#include "imgui.h"
#include "SuperPacker.h"


int main(int argc, char** argv)
{
	OpenGLContext::Init();
	

	
	while (!OpenGLContext::ShouldClose()) {		
		OpenGLContext::BeginFrame();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("file")) {
				if (ImGui::MenuItem("Close"))
				{
					OpenGLContext::request_close();
				}
				ImGui::EndMenu();
			}


			
			ImGui::EndMainMenuBar();
		}

		int size_x, size_y;
		OpenGLContext::GetWindowSize(size_x, size_y);

		ImGui::SetNextWindowPos(ImVec2(0, 20));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(size_x), static_cast<float>(size_y) - 20));
		
		if (ImGui::Begin("background_window", nullptr, ImGuiWindowFlags_NoDecoration))
		{
			SuperPacker::draw_ui();
		}
		ImGui::End();
		
		OpenGLContext::EndFrame();
	}

	OpenGLContext::Shutdown();
}
