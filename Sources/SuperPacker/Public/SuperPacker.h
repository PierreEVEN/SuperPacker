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

		ImagePacker(const std::string& config_path, const std::vector<std::pair<std::string, std::string>>& in_formats, const std::vector<ChannelInfo>& in_channel_infos, const std::vector<ChannelConfiguration>& in_channels_config);
		
		std::optional<std::filesystem::path> pick_file(const std::string& destination);
		std::optional<std::filesystem::path> save_file();

		void draw_ui();

		void save(std::string file_path);

		void draw_channel(ChannelData& channel);

	private:

		std::vector<char> formats;
		std::vector<ChannelInfo> channel_infos;
		std::vector<ChannelConfiguration> channels_config;
		
		Extension export_extension = Extension::EXT_PNG;
		int current_chan_conf = 2;

		std::shared_ptr<IniLoader> config_ini;
	};

	
}
