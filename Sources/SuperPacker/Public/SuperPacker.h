#pragma once
#include <string>
#include <optional>
#include <filesystem>
#include <unordered_map>


#include "Types.h"

class IniLoader;

namespace SuperPacker
{

	class ImagePacker final
	{
	public:

		ImagePacker(const std::string& config_path, const std::optional<std::filesystem::path>& default_image = std::optional<std::filesystem::path>());

		void draw_ui();

		void add_format(const FileFormat& format);
		void add_channel(const ImageChannel& channel);
		void add_channel_combination(const ChannelCombination& combination);
		void set_current_channel_combination(const std::string& combination);
		void set_current_export_format(const std::string& format);
		[[nodiscard]] ChannelCombination& get_combination() { return channel_combinations[current_channel_combination]; }
	
	private:

		std::unordered_map<std::string, FileFormat> formats;
		std::vector<char> formats_string;
		std::unordered_map<std::string, ImageChannel> channels;
		std::unordered_map<std::string, ChannelCombination> channel_combinations;
		std::string current_channel_combination;
		std::string current_export_format;
		
		void draw_channel(ImageChannel& channel);



		void save(std::string file_path);
		void reset_from_source(const std::filesystem::path& source);

		std::shared_ptr<IniLoader> config_ini;
	};
}
