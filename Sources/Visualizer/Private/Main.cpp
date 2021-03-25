

#include "OpenGLContext.h"
#include "imgui.h"

int main(int argc, char** argv)
{
	OpenGLContext::Init();
	

	while (!OpenGLContext::ShouldClose()) {		
		OpenGLContext::BeginFrame();

		if (ImGui::BeginMainMenuBar())
		{



			
			ImGui::EndMainMenuBar();
		}


		
		OpenGLContext::EndFrame();

	}

	OpenGLContext::Shutdown();
}
