#pragma once
#include <string>
#include <cstdint>
#include <vector>

struct ImageData
{
	ImageData(uint8_t* SourceData, uint32_t SourceSizeX, uint32_t SourceSizeY, uint32_t SourceChannels)
	: BaseData(SourceData), SizeX(SourceSizeX), SizeY(SourceSizeY), Channels(SourceChannels) {
		Data = new uint8_t[SizeX * SizeY * Channels];
		std::memcpy(Data, SourceData, SizeX * SizeY * Channels);
	}

	~ImageData()
	{
		delete Data;
	}

	uint8_t* BaseData;
	uint8_t* Data;
	const uint32_t SizeX;
	const uint32_t SizeY;
	const uint32_t Channels;
};

class ImageViewer
{
public:
	ImageViewer();
	static void DisplayAll();
	void LoadFromFile(const std::string& FilePath);
	
	void UpdateBaseData(uint8_t* Data, int sX, int sY, int inChannels);
	
	void ApplyTransformation();

private:
	void Display();

	bool bIsTextureInitialized = false;
	uint32_t TextureID;
	uint8_t* ImageBaseData = nullptr;
	uint64_t ImageID;
	bool bDisplay = true;

	int SizeX = 0, SizeY = 0, Channels = 0;
	
	void DrawMenuBar();
		
	size_t ModifierCount = 0;

	template<typename ModifierClass>
	void AddModifier(const std::string& Name)	{
		ApplyTransformation();
	}
	
};
