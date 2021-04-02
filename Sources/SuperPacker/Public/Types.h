#pragma once
#include <filesystem>
#include <string>
#include <vector>


#include "Image.h"
#include "imgui.h"

namespace SuperPacker
{
	struct ChannelData;

	struct FileFormat
	{
		std::string description;
		std::string name;
	};

	struct ImageChannel
	{
		uint8_t channel_offset;
		std::string full_name;
		std::string short_name;
		ImVec4 channel_color;
		uint8_t default_value;
		std::shared_ptr<IImage> assigned_image;		
	};

	struct ChannelCombination
	{
		std::string combination_name;
		std::vector<std::string> combination;
	};
}
