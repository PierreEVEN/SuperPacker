#pragma once
#include <string>
#include <optional>
#include <filesystem>

#include "GL/gl3w.h"
#include "imgui.h"

namespace SuperPacker
{
	struct ChannelInfo
	{
		std::string channel_name;
		ImVec4 channel_color;
	};

	enum class Extension
	{
		EXT_PNG,
		EXT_TGA,
		EXT_JPG,
		EXT_BMP,
		EXT_HDR
	};


	inline std::string extension_to_string(Extension ext)
	{
		switch (ext)
		{
		case Extension::EXT_PNG:
			return "png";
		case Extension::EXT_TGA:
			return "tga";
		case Extension::EXT_JPG:
			return "jpg";
		case Extension::EXT_BMP:
			return "bmp";
		case Extension::EXT_HDR:
			return "hdr";
		}
		return "none";
	}

	struct Image
	{
		explicit Image(const std::filesystem::path& source_path);
		~Image();

		
		std::vector<std::vector<uint8_t>> data;

		int width = 0;
		int height = 0;
		int channels = 0;

		GLuint texture_id = 0;
	};

	
	struct ChannelData
	{		
		std::filesystem::path source_path;
		std::shared_ptr<Image> image;
		uint8_t channel_id = 0;
		uint8_t desired_channel = 0;
	};

	struct ChannelConfiguration
	{
		std::string configuration_name;
		std::vector<ChannelData> channels;
	};
	
	std::optional<std::filesystem::path> pick_file(const std::string& destination);
	std::optional<std::filesystem::path> save_file();

	void draw_ui();

	void save(const std::string& file_path);

	void draw_channel(ChannelData& channel);
}
