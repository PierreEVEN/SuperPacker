#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>

#include "Types.h"

class IniLoader;

namespace SuperPacker
{

	class ImagePacker final
	{
	public:

		ImagePacker(const std::string& config_path);

		void draw_ui();

		void add_format(const FileFormat& format);
		void add_channel(const ImageChannel& channel);
		void add_channel_combination(const ChannelCombination& combination);
		[[nodiscard]] ChannelCombination& get_combination() { return channel_combinations[current_channel_combination]; }

		void reset_from_source(const std::filesystem::path& source);

		void drop_file(const std::filesystem::path& path);
	private:

		std::vector<std::filesystem::path> dropped_files;
		uint32_t drop_frame = 0;
		
		std::unordered_map<std::string, FileFormat> formats;
		std::vector<char> formats_string;
		std::unordered_map<std::string, ImageChannel> channels;
		std::unordered_map<std::string, ChannelCombination> channel_combinations;
		std::string current_channel_combination;
		std::string current_export_format;
		
		void draw_channel(ImageChannel& channel, const float width);

		void update_preview();
		
		std::shared_ptr<IImage> preview_image;

		void save(std::string file_path);

		std::shared_ptr<IniLoader> config_ini;
	};
}
