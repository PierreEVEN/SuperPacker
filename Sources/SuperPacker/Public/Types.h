#pragma once
#include <filesystem>
#include <string>
#include <vector>


#include "Image.h"
#include "imgui.h"

namespace SuperPacker
{
	struct ChannelData;

	struct PaletteConfiguration
	{
		std::string palette_name;
		std::vector<ChannelData> channels;
	};


	struct ChannelData
	{
		std::filesystem::path source_path;
		std::shared_ptr<Image> image;
		uint8_t channel_id = 0;
		uint8_t desired_channel = 0;
	};


	struct ChannelInfo
	{
		std::string channel_name;
		ImVec4 channel_color;
		uint8_t default_value;
	};

	enum class Extension
	{
		EXT_PNG = 0,
		EXT_TGA = 1,
		EXT_JPG = 2,
		EXT_BMP = 3,
		EXT_HDR = 4,
		EXT_MAX = 5
	};


	inline std::string extension_to_string(Extension ext)
	{
		switch (ext)
		{
		case Extension::EXT_PNG:
			return ".png";
		case Extension::EXT_TGA:
			return ".tga";
		case Extension::EXT_JPG:
			return ".jpg";
		case Extension::EXT_BMP:
			return ".bmp";
		case Extension::EXT_HDR:
			return ".hdr";
		}
		return ".none";
	}
}
