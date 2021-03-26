
#include <windows.h>


#include "SuperPacker.h"

#include <iostream>
#include <vector>


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace SuperPacker
{
	ChannelInfo channel_info[] = {
		{
			.channel_name = "R",
			.channel_color = ImVec4(1, 0.3f, 0.3f, 1)
		},
		{
			.channel_name = "G",
			.channel_color = ImVec4(0.1f, 0.6f, 0.1f, 1)
		},
		{
			.channel_name = "B",
			.channel_color = ImVec4(0.5f, 0.5f, 1, 1)
		},
		{
			.channel_name = "A",
			.channel_color = ImVec4(0.5f, 0.5f, 0.5f, 1)
		}
	};

	
	std::vector<ChannelConfiguration> channels_config = 
	{
		ChannelConfiguration {
			.configuration_name = "Grayscale",
			.channels = {
				{
					.channel_id = 0
				}
			},
		},
		ChannelConfiguration {
			.configuration_name = "RGB",
			.channels = {
				{
					.channel_id = 0
				},
				{
					.channel_id = 1
				},
				{
					.channel_id = 2
				}
			},
		},
		ChannelConfiguration {
			.configuration_name = "RGBA",
			.channels = {
				{
					.channel_id = 0
				},
				{
					.channel_id = 1
				},
				{
					.channel_id = 2
				},
				{
					.channel_id = 3
				}
			},
		},
		
	};

	const char* formats = "PNG file\0*.png\0TGA file\0*.tga\0Jpeg file\0*.jpg\0Bitmap file\0*.bmp\0HDR file\0*.hdr\0Any File\0*.*\0";
	Extension export_extension = Extension::EXT_PNG;
	int current_chan_conf = 2;
	

	Image::Image(const std::filesystem::path& source_path)
	{
		stbi_uc* raw_data = stbi_load(source_path.string().c_str(), &width, &height, &channels, 4);
		
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw_data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		data.resize(4);

		for (int c = 0; c < 4; ++c)
		{
			auto& channel = data[c];
			channel.resize(width * height);

			for (int i = 0; i < width * height; ++i)
			{
				channel[i] = raw_data[i * 4 + c];
			}
		}
		
	}

	Image::~Image()
	{
		glDeleteTextures(1, &texture_id);
	}

	std::optional<std::filesystem::path> pick_file(const std::string& destination)
	{
		char filename[MAX_PATH];
		std::string title = ("Select file for channel " + destination).c_str();

		OPENFILENAME ofn;
		ZeroMemory(&filename, sizeof(filename));
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = formats;
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
			}
		}
		return std::optional<std::filesystem::path>();
	}

	std::optional<std::filesystem::path> save_file()
	{
		char filename[MAX_PATH];
		
		//Save Dialog
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ZeroMemory(&filename, sizeof(filename));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = formats;
		ofn.lpstrFile = filename;
		ofn.lpstrTitle = "Save as...";
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
		
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
			}
		}

		return std::optional<std::filesystem::path>();
	}
	
	void save(const std::string& file_path)
	{
		uint32_t width = 0;
		uint32_t height = 0;

		for (const auto& channel : channels_config[current_chan_conf].channels)
		{
			if (!channel.image) continue;
			if (width && width != channel.image->width || height && height != channel.image->height)
			{
				std::cout << "wrong dimension in channel " << channel.channel_id << std::endl;
				return;
			}
			width = channel.image->width;
			height = channel.image->height;
		}
		
		if (!width || !height)
		{
			std::cout << "invalid image dimensions" << std::endl;
			return;
		}
		
		std::vector<stbi_uc> combined_data;
		combined_data.resize(channels_config[current_chan_conf].channels.size() * width * height);
				
		for (size_t i = 0; i < width * height; ++i)
		{
			for (int chan_id = 0; chan_id < channels_config[current_chan_conf].channels.size(); ++chan_id)
			{

				uint8_t chan_color = 0;
				if (channels_config[current_chan_conf].channels[chan_id].image)
				{
					auto& chan_data = channels_config[current_chan_conf].channels[chan_id];
					chan_color = chan_data.image->data[chan_data.desired_channel][i];
				}
				
				combined_data[i * channels_config[current_chan_conf].channels.size() + chan_id] = chan_color;
			}
		}


		std::cout << "write image : " << width << " x " << height << " with " << channels_config[current_chan_conf].channels.size() << " components" << std::endl;

		switch (export_extension)
		{
		case Extension::EXT_TGA:
			stbi_write_tga(file_path.c_str(), width, height, static_cast<int>(channels_config[current_chan_conf].channels.size()), combined_data.data());
			break;
		case Extension::EXT_PNG:
			stbi_write_png(file_path.c_str(), width, height, static_cast<int>(channels_config[current_chan_conf].channels.size()), combined_data.data(), 0);
			break;
		case Extension::EXT_BMP:
			stbi_write_bmp(file_path.c_str(), width, height, static_cast<int>(channels_config[current_chan_conf].channels.size()), combined_data.data());
			break;
		case Extension::EXT_JPG:
			stbi_write_jpg(file_path.c_str(), width, height, static_cast<int>(channels_config[current_chan_conf].channels.size()), combined_data.data(), 100);
			break;
		case Extension::EXT_HDR:
			std::cout << "not implemented yet";
			break;
		}
	}

	void draw_ui()
	{
		ImGui::Columns(static_cast<int>(channels_config[current_chan_conf].channels.size()));
		for (auto& channel : channels_config[current_chan_conf].channels)
		{
			draw_channel(channel);
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::Separator();
		if (ImGui::BeginCombo("channel configuration", channels_config[current_chan_conf].configuration_name.c_str()))
		{
			for (int i = 0; i < channels_config.size(); ++i) {
				if (ImGui::MenuItem(channels_config[i].configuration_name.c_str())) current_chan_conf = i;
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("format", extension_to_string(export_extension).c_str()))
		{
			if (ImGui::MenuItem(extension_to_string(Extension::EXT_BMP).c_str())) export_extension = Extension::EXT_BMP;
			if (ImGui::MenuItem(extension_to_string(Extension::EXT_JPG).c_str())) export_extension = Extension::EXT_JPG;
			if (ImGui::MenuItem(extension_to_string(Extension::EXT_PNG).c_str())) export_extension = Extension::EXT_PNG;
			if (ImGui::MenuItem(extension_to_string(Extension::EXT_TGA).c_str())) export_extension = Extension::EXT_TGA;
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
	
	void draw_channel(ChannelData& channel)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, channel_info[channel.channel_id].channel_color);
		if (ImGui::Button((channel_info[channel.channel_id].channel_name).c_str()))
		{
			if (auto path = pick_file(channel_info[channel.channel_id].channel_name)) {
				channel.source_path = path.value();
				channel.image = std::make_shared<Image>(channel.source_path);
			}
		}
		ImGui::PopStyleColor();
		if (channel.image) {
			ImGui::SameLine();
			if (ImGui::Button(("remove##" + channel_info[channel.channel_id].channel_name).c_str()))
			{
				channel.image = nullptr;
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
			ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<size_t>(channel.image->texture_id)), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1));

			if (channel.desired_channel > channel.image->channels) channel.desired_channel = 0;
			ImGui::PushStyleColor(ImGuiCol_Text, channel_info[channel.desired_channel].channel_color);
			if (ImGui::BeginCombo(("##" + channel_info[channel.channel_id].channel_name).c_str(), channel_info[channel.desired_channel].channel_name.c_str()))
			{
				for (int i = 0; i < channel.image->channels; ++i)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, channel_info[i].channel_color);
					if (ImGui::MenuItem(channel_info[i].channel_name.c_str()))
					{
						channel.desired_channel = static_cast<uint8_t>(i);
					}
					ImGui::PopStyleColor();
				}
				
				ImGui::EndCombo();
			}
			ImGui::PopStyleColor();
			
		}
	}
}
 