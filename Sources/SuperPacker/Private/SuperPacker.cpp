
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "SuperPacker.h"
#include "ApiInteface.h"
#include "IniLoader.h"
#include "Logger.h"

namespace SuperPacker
{
	ImagePacker::ImagePacker(const std::string& config_path, const std::optional<std::filesystem::path>& default_image)
	{
		config_ini = std::make_shared<IniLoader>(config_path);

		current_export_format = config_ini->get_property_as_string("defaults", "export_extension", "");
		current_channel_combination = config_ini->get_property_as_string("defaults", "export_palette", "");

		if (default_image) {
			reset_from_source(default_image.value());
		}
	}

	void ImagePacker::draw_ui()
	{
		if (ImGui::Button("Pick source")) if (auto file = pick_file("", formats_string)) reset_from_source(file.value());

		auto& combination = channel_combinations[current_channel_combination].combination;
		for (const auto& channel : combination)
		{
			ImGui::Columns(static_cast<int>(combination.size()));
			draw_channel(channels[channel]);
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::Separator();
		
		if (preview_image) ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<size_t>(preview_image->get_texture())), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1));
		
		ImGui::Separator();
		if (channel_combinations.find(current_channel_combination) == channel_combinations.end()) current_channel_combination = channel_combinations.begin()->first;
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

		if (formats.find(current_export_format) == formats.end()) current_export_format = formats.begin()->first;		
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
		ImGui::PushStyleColor(ImGuiCol_Button, channel.channel_color);
		if (ImGui::Button(channel.full_name.c_str())) {
			if (auto path = pick_file("", formats_string)) {
				channel.assigned_image = std::make_shared<Image>(path.value());
				update_preview();
			}
		}
		
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (channel.assigned_image) {
			if (ImGui::Button(("remove##" + channel.full_name).c_str())) channel.assigned_image = nullptr;
		}
		else
		{
			int last_value = channel.default_value;
			ImGui::DragInt(("##default" + channel.full_name).c_str(), &last_value, 1, 0, 255);
			if (last_value != channel.default_value)
			{
				channel.default_value = static_cast<uint8_t>(last_value);
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::Text("%s", channel.assigned_image && channel.assigned_image->source_path ? channel.assigned_image->source_path.value().string().c_str() : "none");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		if (channel.assigned_image) {
			ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<size_t>(channel.assigned_image->get_texture())), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1));


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
						config_ini->set_property_as_string("defaults","palette_" + channels[comb_chan].short_name, channel.desired_channel);
						update_preview();
					}
					ImGui::PopStyleColor();
				}

				ImGui::EndCombo();
			}
			ImGui::PopStyleColor();

		}
	}

	void ImagePacker::update_preview()
	{
		for (const auto& channel : channel_combinations[current_channel_combination].combination)
		{
			auto& assigned_image = channels[channel].assigned_image;
			if (assigned_image && (!preview_image || preview_image->get_width() != assigned_image->get_width() || preview_image->get_height() != assigned_image->get_height()))
			{
				preview_image = std::make_shared<Image>(assigned_image->get_width(), assigned_image->get_height(), static_cast<int>(channel_combinations[current_channel_combination].combination.size()));
			}

			if (assigned_image)
			{
				static_cast<Image*>(preview_image.get())->set_channel_data(
					static_cast<Image*>(assigned_image.get())->get_channel_data(channels[channels[channel].desired_channel].channel_offset),
					channels[channel].channel_offset
				);
			}
			
		}
		if (preview_image) static_cast<Image*>(preview_image.get())->rebuild_texture();
		
	}

	void ImagePacker::save(std::string file_path)
	{
		int width = 0;
		int height = 0;

			
		auto& channel_combination = channel_combinations[current_channel_combination];
		
		for (const auto& channel_comb : channel_combination.combination) {
			auto& channel = channels[channel_comb];
			if (!channel.assigned_image) continue;
			
			if (width && width != channel.assigned_image->get_width() || height && height != channel.assigned_image->get_height())
			{
				logger_error("wrong dimension in channel %s", channel.full_name.c_str());
				return;
			}
			width = channel.assigned_image->get_width();
			height = channel.assigned_image->get_height();
		}
		
		if (!width || !height)
		{
			logger_error("invalid image dimensions");
			return;
		}
				
		std::vector<uint8_t> combined_data;
		size_t pixel_count = width * height;
		combined_data.resize(channel_combination.combination.size() * pixel_count);
				
		for (size_t i = 0; i < pixel_count; ++i)
		{
			for (const auto& channel_comb : channel_combination.combination)
			{
				auto& channel = channels[channel_comb];
				uint8_t chan_color = channel.default_value;
				if (channel.assigned_image)
				{
					if (auto img = static_cast<Image*>(channel.assigned_image.get())) {

						chan_color = img->get_pixel(channel.channel_offset, i);
					}
				}
				
				combined_data[i * channel_combination.combination.size() + channel.channel_offset] = chan_color;
			}
		}

		std::string extension = "." + formats[current_export_format].name;

		for (int64_t i = extension.size() - 1; i >= 0; --i) {
			if (extension[i] != file_path[file_path.size() - 1 - i]) {
				file_path += extension;
				break;
			}
		}

		if (current_export_format == "png")
		{
			stbi_write_png(file_path.c_str(), width, height, static_cast<int>(channel_combination.combination.size()), combined_data.data(), 0);
		}
		else if (current_export_format == "tga")
		{
			stbi_write_tga(file_path.c_str(), width, height, static_cast<int>(channel_combination.combination.size()), combined_data.data());			
		}
		else if (current_export_format == "bmp")
		{
			stbi_write_bmp(file_path.c_str(), width, height, static_cast<int>(channel_combination.combination.size()), combined_data.data());
		}
		else if (current_export_format == "jpg") {
			stbi_write_jpg(file_path.c_str(), width, height, static_cast<int>(channel_combination.combination.size()), combined_data.data(), 100);
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
}