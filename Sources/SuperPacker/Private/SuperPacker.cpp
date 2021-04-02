
#include <windows.h>
#include <iostream>
#include <vector>

#include "SuperPacker.h"



#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "IniLoader.h"
#include "Logger.h"


namespace SuperPacker
{
	ImagePacker::ImagePacker(const std::string& config_path, const std::vector<std::pair<std::string, std::string>>& in_formats,
		const std::vector<ChannelInfo>& in_channel_infos, const std::vector<PaletteConfiguration>& in_channels_config, 
		const std::optional<std::filesystem::path>& default_image) :
	channel_infos(in_channel_infos),
	palette_config(in_channels_config)
	{
		for (const auto& format : in_formats)
		{
			for (const auto& chr : format.first) formats.push_back(chr);
			formats.push_back('\0');
			for (const auto& chr : format.second) formats.push_back(chr);
			formats.push_back('\0');
		}
		formats.push_back('\0');

		config_ini = std::make_shared<IniLoader>(config_path);
		
		export_extension = static_cast<Extension>(config_ini->GetPropertyAsInt("defaults", "export_extension", 0));
		export_palette = config_ini->GetPropertyAsInt("defaults", "export_palette", 2);

		if (default_image) {
			reset_from_source(default_image.value());
		}

		for (auto& palette : palette_config)
		{
			for (int i = 0; i < palette.channels.size(); ++i)
			{
				palette.channels[i].desired_channel = config_ini->GetPropertyAsInt(("palette_" + palette.palette_name).c_str(), ("palette_" + palette.palette_name + "_" + std::to_string(i)).c_str(), 0);
				channel_infos[i].default_value = config_ini->GetPropertyAsInt(("palette_" + palette.palette_name).c_str(), ("default_" + palette.palette_name + "_" + std::to_string(i)).c_str(), channel_infos[i].default_value);
			}
		}
	}

	std::optional<std::filesystem::path> ImagePacker::pick_file(const std::string& destination)
	{
		char filename[MAX_PATH];
		std::string title = ("Select file for channel " + destination).c_str();

		OPENFILENAME ofn;
		ZeroMemory(&filename, sizeof(filename));
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = formats.data();
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = title.c_str();
		ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&ofn))
		{
			return std::filesystem::path(filename);
		}
		else
		{
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
			case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
			case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
			case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
			case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
			case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
			case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
			case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
			case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
			case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
			case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
			case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
			case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
			case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
			case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
			default: break;
			}
		}
		return std::optional<std::filesystem::path>();
	}

	std::optional<std::filesystem::path> ImagePacker::save_file()
	{
		char filename[MAX_PATH];
		
		//Save Dialog
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ZeroMemory(&filename, sizeof(filename));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = formats.data();
		ofn.lpstrFile = filename;
		ofn.lpstrTitle = "Save as...";
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
		
		if (GetSaveFileName(&ofn))
		{
			return std::filesystem::path(filename);
		}
		else
		{
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
			case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
			case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
			case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
			case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
			case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
			case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
			case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
			case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
			case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
			case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
			case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
			case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
			case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
			case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
			}
		}

		return std::optional<std::filesystem::path>();
	}
	
	void ImagePacker::save(std::string file_path)
	{
		uint32_t width = 0;
		uint32_t height = 0;

		for (const auto& channel : palette_config[export_palette].channels)
		{
			if (!channel.image) continue;
			if (width && width != channel.image->get_width() || height && height != channel.image->get_height())
			{
				std::cout << "wrong dimension in channel " << channel.channel_id << std::endl;
				return;
			}
			width = channel.image->get_width();
			height = channel.image->get_height();
		}
		
		if (!width || !height)
		{
			std::cout << "invalid image dimensions" << std::endl;
			return;
		}
		
		std::vector<stbi_uc> combined_data;
		combined_data.resize(palette_config[export_palette].channels.size() * width * height);
				
		for (size_t i = 0; i < width * height; ++i)
		{
			for (int chan_id = 0; chan_id < palette_config[export_palette].channels.size(); ++chan_id)
			{

				uint8_t chan_color = channel_infos[chan_id].default_value;
				if (palette_config[export_palette].channels[chan_id].image)
				{
					auto& chan_data = palette_config[export_palette].channels[chan_id];
					
					if (auto img = std::dynamic_pointer_cast<Image>(chan_data.image)) {

						chan_color = img->get_pixel(chan_data.desired_channel, i);
					}
				}
				
				combined_data[i * palette_config[export_palette].channels.size() + chan_id] = chan_color;
			}
		}

		std::string extension = extension_to_string(export_extension);

		for (int64_t i = extension.size() - 1; i >= 0; --i)
		{
			if (extension[i] != file_path[file_path.size() - 1 - i])
			{
				file_path += extension;
				break;
			}
		}
		

		std::cout << "write image : " << width << " x " << height << " with " << palette_config[export_palette].channels.size() << " components" << std::endl;

		switch (export_extension)
		{
		case Extension::EXT_TGA:
			stbi_write_tga(file_path.c_str(), width, height, static_cast<int>(palette_config[export_palette].channels.size()), combined_data.data());
			break;
		case Extension::EXT_PNG:
			stbi_write_png(file_path.c_str(), width, height, static_cast<int>(palette_config[export_palette].channels.size()), combined_data.data(), 0);
			break;
		case Extension::EXT_BMP:
			stbi_write_bmp(file_path.c_str(), width, height, static_cast<int>(palette_config[export_palette].channels.size()), combined_data.data());
			break;
		case Extension::EXT_JPG:
			stbi_write_jpg(file_path.c_str(), width, height, static_cast<int>(palette_config[export_palette].channels.size()), combined_data.data(), 100);
			break;
		case Extension::EXT_HDR:
			std::cout << "not implemented yet";
			break;
		}
	}

	void ImagePacker::draw_ui()
	{
		if (ImGui::Button("Pick source"))
		{
			if (auto file = pick_file(""))
			{
				reset_from_source(file.value());
			}
		}
		
		ImGui::Columns(static_cast<int>(palette_config[export_palette].channels.size()));
		for (size_t i = 0; i < palette_config[export_palette].channels.size(); ++i)
		{
			draw_channel(palette_config[export_palette].channels[i], static_cast<int>(i));
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::Separator();
		if (ImGui::BeginCombo("output palette", palette_config[export_palette].palette_name.c_str()))
		{
			for (int i = 0; i < palette_config.size(); ++i) {
				if (ImGui::MenuItem(palette_config[i].palette_name.c_str())) {
					export_palette = i;
					config_ini->SetPropertyAsInt("defaults", "export_palette", i);
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("format", extension_to_string(export_extension).c_str()))
		{
			for (int i = 0; i < static_cast<int>(Extension::EXT_MAX); ++i)
			{

				if (ImGui::MenuItem(extension_to_string(static_cast<Extension>(i)).c_str())) {
					export_extension = static_cast<Extension>(i);
					config_ini->SetPropertyAsInt("defaults", "export_extension", static_cast<int>(export_extension));
					config_ini->SetPropertyAsInt(
						("palette_" + palette_config[export_palette].palette_name).c_str(),
						("default_" + palette_config[export_palette].palette_name + "_" + std::to_string(i)).c_str(), i);
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Export"))
		{
			if (auto file = save_file())
			{
				save(file->string());
			}
		}
	}
	
	void ImagePacker::draw_channel(ChannelData& channel, int channel_id)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, channel_infos[channel.channel_id].channel_color);
		if (ImGui::Button((channel_infos[channel.channel_id].channel_name).c_str()))
		{
			if (auto path = pick_file(channel_infos[channel.channel_id].channel_name)) {
				channel.source_path = path.value();
				channel.image = std::make_shared<Image>(channel.source_path);
			}
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (channel.image) {
			if (ImGui::Button(("remove##" + channel_infos[channel.channel_id].channel_name).c_str()))
			{
				channel.image = nullptr;
			}
		}
		else
		{
			int last_value = channel_infos[channel_id].default_value;
			ImGui::DragInt(("##default" + channel_infos[channel.channel_id].channel_name).c_str(), &last_value, 1, 0, 255);
			if (last_value != channel_infos[channel_id].default_value)
			{
				channel_infos[channel_id].default_value = static_cast<uint8_t>(last_value);
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::Text("%s", channel.image ? channel.source_path.string().c_str() : "none");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		if (channel.image) {
			ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<size_t>(channel.image->get_texture())), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1));

			if (channel.desired_channel > channel.image->get_channels()) channel.desired_channel = 0;
			ImGui::PushStyleColor(ImGuiCol_Text, channel_infos[channel.desired_channel].channel_color);
			if (ImGui::BeginCombo(("##" + channel_infos[channel.channel_id].channel_name).c_str(), channel_infos[channel.desired_channel].channel_name.c_str()))
			{
				for (int i = 0; i < channel.image->get_channels(); ++i)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, channel_infos[i].channel_color);
					if (ImGui::MenuItem(channel_infos[i].channel_name.c_str()))
					{
						channel.desired_channel = static_cast<uint8_t>(i);
						config_ini->SetPropertyAsInt(
							("palette_" + palette_config[export_palette].palette_name).c_str(),
							("palette_" + palette_config[export_palette].palette_name + "_" + std::to_string(channel_id)).c_str(), i);
					}
					ImGui::PopStyleColor();
				}
				
				ImGui::EndCombo();
			}
			ImGui::PopStyleColor();
			
		}
	}

	void ImagePacker::reset_from_source(const std::filesystem::path& source)
	{
		for (auto& channel : palette_config[export_palette].channels)
		{
			channel.image = std::make_shared<Image>(source);
			channel.source_path = source;
		}
	}
}
 