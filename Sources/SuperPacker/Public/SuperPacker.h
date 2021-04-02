#pragma once
#include <string>
#include <optional>
#include <filesystem>

#include "Types.h"

class IniLoader;

namespace SuperPacker
{

	class ImagePacker final
	{
	public:

		ImagePacker(const std::string& config_path, const std::vector<std::pair<std::string, std::string>>& in_formats, const std::vector<ChannelInfo>& in_channel_infos, const std::vector<PaletteConfiguration>& in_channels_config, const std::optional<std::filesystem::path>& default_image = std::optional<std::filesystem::path>());
		
		void draw_ui();

	private:

		std::optional<std::filesystem::path> pick_file(const std::string& destination);
		std::optional<std::filesystem::path> save_file();

		void save(std::string file_path);

		void draw_channel(ChannelData& channel, int channel_id);

		void reset_from_source(const std::filesystem::path& source);

		std::vector<char> formats;
		std::vector<ChannelInfo> channel_infos;
		std::vector<PaletteConfiguration> palette_config;
		
		Extension export_extension = Extension::EXT_PNG;
		int export_palette = 2;

		std::shared_ptr<IniLoader> config_ini;
	};

	
}
