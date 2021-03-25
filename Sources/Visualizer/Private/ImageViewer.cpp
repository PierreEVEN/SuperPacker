

#include "ImageViewer.h"

#include "imgui.h"
#include "OpenGLContext.h"

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <filesystem>


#include "stb_image.h"
#include "GL/gl3w.h"
#include "Modifiers/Clamp.h"
#include "Modifiers/Close.h"
#include "Modifiers/Open.h"
#include "Modifiers/Convolution.h"
#include "Modifiers/Dilate.h"
#include "Modifiers/Equalizer.h"
#include "Modifiers/Erode.h"
#include "Modifiers/ExtractBorder.h"
#include "Modifiers/Histogram.h"
#include "Modifiers/MedianFilter.h"
#include "Modifiers/Negate.h"
#include "Modifiers/StretchContrast.h"
#include "Modifiers/Threshold.h"


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
		if (ImGui::BeginMenu("Add modifier")) {
			if (ImGui::MenuItem("Negate")) AddModifier<NegateModifier>("Negative");
			if (ImGui::MenuItem("Clamp")) AddModifier<ClampModifier>("Clamp");
			if (ImGui::MenuItem("Threshold")) AddModifier<ThresholdModifier>("Threshold");
			if (ImGui::MenuItem("Normalize")) AddModifier<NormalizerModifier>("Normalize");
			if (ImGui::MenuItem("Histogram")) AddModifier<Histogram>("Histogram");
			if (ImGui::MenuItem("Equalizer")) AddModifier<Equalizer>("Equalizer");
			if (ImGui::MenuItem("Convolution")) AddModifier<ConvolutionModifier>("Convolution");
			if (ImGui::MenuItem("Median filter")) AddModifier<MedianFilterModifier>("Median Filter");
			if (ImGui::MenuItem("Dilate")) AddModifier<DilateModifier>("Dilate");
			if (ImGui::MenuItem("Erode")) AddModifier<ErodeFilter>("Erode");
			if (ImGui::MenuItem("Open")) AddModifier<OpenModifier>("Open");
			if (ImGui::MenuItem("Close")) AddModifier<CloseModifier>("Close");
			if (ImGui::MenuItem("Extract border")) AddModifier<ExtractBorderFilter>("Extract border");
			ImGui::EndMenu();
		}
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
		for (const auto& Modifier : Modifiers) {
			Modifier->DrawUI_Internal();
		}	
	}
	ImGui::End();
}

void ImageViewer::ApplyTransformation() {
	std::cout << "rebuild image" << std::endl;

	if (!ImageBaseData || SizeX * SizeY * Channels <= 0) return;
	
	ImageData ModifiedData(ImageBaseData, SizeX, SizeY, Channels);

	for (const auto& Modifier : Modifiers)
	{
		Modifier->ModifyImage_Internal(&ModifiedData);
	}
	
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
