

#include "ImageViewer.h"

#include "imgui.h"
#include "OpenGLContext.h"

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <filesystem>


#include "stb_image.h"
#include "GL/gl3w.h"


std::vector <ImageViewer*> ImagesViewers;
uint64_t ImageID = 0;

void ImageViewer::DisplayAll() {
	// Draw all instances
	for (int64_t i = ImagesViewers.size() - 1; i >= 0; --i)
	{
		if (!ImagesViewers[i]->bDisplay)
		{
			delete ImagesViewers[i];
			ImagesViewers.erase(ImagesViewers.begin() + i);
			continue;
		}
		ImagesViewers[i]->Display();
	}
}

ImageViewer::ImageViewer() {
	// Register instance
	ImagesViewers.push_back(this);
	ImageID = ImageID++;
}

void ImageViewer::DrawMenuBar() {
	if (ImGui::BeginMenuBar()) {
		ImGui::EndMenuBar();
	}
}

void ImageViewer::UpdateBaseData(uint8_t* Data, int sX, int sY, int inChannels) {
	if (ImageBaseData) free(ImageBaseData);

	ImageBaseData = Data;
	SizeX = sX;
	SizeY = sY;
	Channels = inChannels;
	
	ApplyTransformation();
}

void ImageViewer::LoadFromFile(const std::string& FilePath) {	
	std::cout << "update image data" << std::endl;
	int sX, sY, newChannels;
	stbi_uc* NewImageData = stbi_load(FilePath.c_str(), &sX, &sY, &newChannels, 1);
	if (!NewImageData)	{
		std::cerr << "failed to load image from " << FilePath << std::endl;
		return;
	}
	
	UpdateBaseData(NewImageData, sX, sY, 1);
}


void ImageViewer::Display() {
	if (!bDisplay) return;
	
	if (ImGui::Begin(("Image " + std::to_string(ImageID)).c_str(), &bDisplay, ImGuiWindowFlags_MenuBar)) {
		DrawMenuBar();
		ImGui::Image(reinterpret_cast<void*>(static_cast<uint64_t>(TextureID)), ImVec2(static_cast<float>(SizeX), static_cast<float>(SizeY)));
	}
	ImGui::End();
}

void ImageViewer::ApplyTransformation() {
	std::cout << "rebuild image" << std::endl;

	if (!ImageBaseData || SizeX * SizeY * Channels <= 0) return;
	
	ImageData ModifiedData(ImageBaseData, SizeX, SizeY, Channels);
	
	
	uint8_t* TempData = new uint8_t[SizeX * SizeY * 4];
	for (size_t i = 0; i < SizeX * SizeY; ++i) {
		TempData[i * 4] = ModifiedData.Data[i];
		TempData[i * 4 + 1] = ModifiedData.Data[i];
		TempData[i * 4 + 2] = ModifiedData.Data[i];
	}

	if (!bIsTextureInitialized) glGenTextures(1, &TextureID);
	bIsTextureInitialized = true;
	glBindTexture(GL_TEXTURE_2D, TextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SizeX, SizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, TempData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (TempData) free(TempData);
}
