
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "SuperPacker.h"
#include "ApiInteface.h"
#include "IniLoader.h"
#include "Logger.h"

namespace SuperPacker
{
	ImagePacker::ImagePacker(const std::string& config_path)
	{
		config_ini = std::make_shared<IniLoader>(config_path);

		current_export_format = config_ini->get_property_as_string("defaults", "export_extension", "");
		current_channel_combination = config_ini->get_property_as_string("defaults", "export_palette", "");
	}

	void add_tooltip(const std::string& text)
	{
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::Text("%s", text.c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void ImagePacker::draw_ui()
	{
		if (channel_combinations.find(current_channel_combination) == channel_combinations.end()) current_channel_combination = channel_combinations.begin()->first;
		if (formats.find(current_export_format) == formats.end()) current_export_format = formats.begin()->first;
		if (ImGui::Button("Pick source")) if (auto file = pick_file("", formats_string)) reset_from_source(file.value());
		add_tooltip("Choose image for all channels");
		
		auto& combination = channel_combinations[current_channel_combination].combination;
		for (const auto& channel : combination)
		{
			ImGui::Columns(static_cast<int>(combination.size()));
			draw_channel(channels[channel]);
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::Separator();
		
		if (preview_image) {
			ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<size_t>(preview_image->get_texture())), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1));
			add_tooltip("output preview");
		}
		
		ImGui::Separator();
		if (ImGui::BeginCombo("output palette", current_channel_combination.c_str()))
		{
			for (const auto& chan_combination : channel_combinations)
			{
				if (ImGui::MenuItem(chan_combination.second.combination_name.c_str())) {
					current_channel_combination = chan_combination.first;
					config_ini->set_property_as_string("defaults", "export_palette", current_channel_combination);
					update_preview();
				}
			}
			ImGui::EndCombo();
		}
	
		if (ImGui::BeginCombo("format", formats[current_export_format].description.c_str()))
		{
			for (const auto& format : formats)
			{
				if (ImGui::MenuItem(format.second.description.c_str())) {
					current_export_format = format.first;
					config_ini->set_property_as_string("defaults", "export_extension", current_export_format);
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Export"))
		{
			std::vector<char> current_format_string;

			for (const auto& chr : formats[current_export_format].description) current_format_string.push_back(chr);
			current_format_string.push_back('\0');
			for (const auto& chr : formats[current_export_format].name) current_format_string.push_back(chr);
			current_format_string.push_back('\0');
			current_format_string.push_back('\0');
			if (auto file = save_file(current_format_string))
			{
				save(file->string());
			}
		}
	}

	void ImagePacker::add_format(const FileFormat& format)
	{
		formats[format.name] = format;

		for (const auto& chr : format.description) formats_string.push_back(chr);
		formats_string.push_back('\0');
		for (const auto& chr : format.name) formats_string.push_back(chr);
		formats_string.push_back('\0');
	}

	void ImagePacker::add_channel(const ImageChannel& channel)
	{
		channels[channel.short_name] = channel;
		channels[channel.short_name].desired_channel = config_ini->get_property_as_string("defaults", "palette_" + channel.short_name);
	}
	void ImagePacker::add_channel_combination(const ChannelCombination& combination)
	{
		channel_combinations[combination.combination_name] = combination;
	}
	
	void ImagePacker::draw_channel(ImageChannel& channel)
	{
		if (ImGui::BeginChild(("channel : " + channel.full_name).c_str(), ImVec2(0, 200))) {

			if (ImGui::IsWindowHovered() && !dropped_files.empty())
			{
				channel.assigned_image = std::make_shared<Image>(dropped_files[0]);
				update_preview();
				dropped_files.clear();
			}
			
			ImGui::PushStyleColor(ImGuiCol_Button, channel.channel_color);
			if (ImGui::Button(channel.full_name.c_str())) {
				if (auto path = pick_file("", formats_string)) {
					channel.assigned_image = std::make_shared<Image>(path.value());
					update_preview();
				}
			}
			add_tooltip(channel.assigned_image ? channel.assigned_image->source_path->string() : "Choose image for the " + channel.full_name + " channel");
			
			ImGui::PopStyleColor();
			ImGui::SameLine();
			if (channel.assigned_image) {
				if (ImGui::Button(("remove##" + channel.full_name).c_str())) {
					channel.assigned_image = nullptr;
					update_preview();
				}
				add_tooltip("Remove image");
			}
			else
			{
				int last_value = channel.default_value;
				ImGui::DragInt(("##default" + channel.full_name).c_str(), &last_value, 1, 0, 255);
				if (last_value != channel.default_value)
				{
					channel.default_value = static_cast<uint8_t>(last_value);
					update_preview();
				}
				add_tooltip("default channel value");
			}
			if (channel.assigned_image) {
				ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<size_t>(channel.assigned_image->get_texture())), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1));

				if (channel.assigned_image->source_path) add_tooltip(channel.assigned_image->source_path->filename().string());

				if (channels.find(channel.desired_channel) == channels.end()) channel.desired_channel = channels.begin()->first;

				auto& desired_channel = channels[channel.desired_channel];
				auto& combination = channel_combinations[current_channel_combination];

				ImGui::PushStyleColor(ImGuiCol_Text, desired_channel.channel_color);
				if (ImGui::BeginCombo(("##" + channel.short_name).c_str(), desired_channel.full_name.c_str()))
				{
					for (const auto& comb_chan : combination.combination)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, channels[comb_chan].channel_color);
						if (ImGui::MenuItem(channels[comb_chan].full_name.c_str()))
						{
							channel.desired_channel = channels[comb_chan].short_name;
							config_ini->set_property_as_string("defaults", "palette_" + channels[comb_chan].short_name, channel.desired_channel);
							update_preview();
						}
						ImGui::PopStyleColor();
					}

					ImGui::EndCombo();
				}
				ImGui::PopStyleColor();

				add_tooltip("Choose output channel from this source");

			}
		}
		ImGui::EndChild();
	}

	void ImagePacker::update_preview()
	{
		const auto channel_count = channel_combinations[current_channel_combination].combination.size();

		int image_width = 0;
		int image_height = 0;

		for (const auto& channel : channel_combinations[current_channel_combination].combination)
		{
			if (channels[channel].assigned_image)
			{
				if (!image_width || !image_height) {
					image_width = channels[channel].assigned_image->get_width();
					image_height = channels[channel].assigned_image->get_height();
				}
				else if (image_width != channels[channel].assigned_image->get_width() || image_height != channels[channel].assigned_image->get_height())
				{
					logger_warning("wrong image dimention");
					preview_image = nullptr;
					return;
				}
			}
		}

		if (image_width == 0 || image_height == 0) {
			preview_image = nullptr;
			return;
		}

		if (!preview_image)	preview_image = std::make_shared<Image>(image_width, image_height, static_cast<int>(channel_count));
		
		for (const auto& channel : channel_combinations[current_channel_combination].combination)
		{
			auto& assigned_image = channels[channel].assigned_image;
			if (assigned_image && (preview_image->get_width() != assigned_image->get_width() || preview_image->get_height() != assigned_image->get_height()) || preview_image->get_channels() != channel_count)
			{
				preview_image = std::make_shared<Image>(assigned_image->get_width(), assigned_image->get_height(), static_cast<int>(channel_count));
			}

			if (assigned_image)
			{
				static_cast<Image*>(preview_image.get())->set_channel_data(
					static_cast<Image*>(assigned_image.get())->get_channel_data(channels[channels[channel].desired_channel].channel_offset),
					channels[channel].channel_offset
				);
			}
			else
			{
				std::vector<uint8_t> image_data(image_width* image_height);
				for (int i = 0; i < image_width * image_height; ++i)
				{
					image_data[i] = channels[channels[channel].desired_channel].default_value;
				}
				static_cast<Image*>(preview_image.get())->set_channel_data(image_data, channels[channel].channel_offset);
			}
			
		}
		if (preview_image) static_cast<Image*>(preview_image.get())->rebuild_texture();
		
	}

	std::string set_extension(const std::string& current_name, const std::string& desired_extension)
	{
		return std::filesystem::path(current_name).parent_path().string() + "/" + std::filesystem::path(current_name).stem().string() + "." + desired_extension;
	}

	
	void ImagePacker::save(std::string file_path)
	{
		int width = 0;
		int height = 0;

		if (!preview_image)
		{
			logger_warning("cannot export current image combination");
			return;
		}

		auto export_path = set_extension(file_path, formats[current_export_format].short_name);

		auto data = static_cast<Image*>(preview_image.get())->gen_data_from_channels(preview_image->get_channels());
		
		if (formats[current_export_format].short_name == "png")
		{
			stbi_write_png(export_path.c_str(), preview_image->get_width(), preview_image->get_height(), static_cast<int>(preview_image->get_channels()), data.data(), 0);
		}
		else if (formats[current_export_format].short_name == "tga")
		{
			stbi_write_tga(export_path.c_str(), preview_image->get_width(), preview_image->get_height(), static_cast<int>(preview_image->get_channels()), data.data());
		}
		else if (formats[current_export_format].short_name == "bmp")
		{
			stbi_write_bmp(export_path.c_str(), preview_image->get_width(), preview_image->get_height(), static_cast<int>(preview_image->get_channels()), data.data());
		}
		else if (formats[current_export_format].short_name == "jpg") {
			stbi_write_jpg(export_path.c_str(), preview_image->get_width(), preview_image->get_height(), static_cast<int>(preview_image->get_channels()), data.data(), 100);
		}
		else
		{
			logger_error("unsuported format : %s", formats[current_export_format].short_name.c_str());
		}
	}
		
	void ImagePacker::reset_from_source(const std::filesystem::path& source)
	{
		for (auto& channel : channels)
		{
			channel.second.assigned_image = std::make_shared<Image>(source);
		}
		update_preview();
	}

	void ImagePacker::drop_file(const std::filesystem::path& path)
	{
		dropped_files.push_back(path);
		logger_log("drop file %s", path.string().c_str());		
	}
}
