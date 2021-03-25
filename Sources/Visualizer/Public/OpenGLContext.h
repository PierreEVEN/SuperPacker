#pragma once

namespace OpenGLContext
{

	void Init();

	bool ShouldClose();
	void BeginFrame();
	void EndFrame();
	void Shutdown();
	void GetWindowSize(int& SizeX, int& SizeY);

	



	
}