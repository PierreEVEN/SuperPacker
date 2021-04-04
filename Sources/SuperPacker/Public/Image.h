#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stb_image.h>


#include "GL/gl3w.h"

namespace SuperPacker {


	class IImage
	{
	public:
		explicit IImage(const std::filesystem::path& path)
			: source_path(path) {}

		explicit IImage(const int in_with, const int in_height, const int in_channels)
			: width(in_with), height(in_height), channels(in_channels) {}
		
		[[nodiscard]] int get_width() const { return width; }
		[[nodiscard]] int get_height() const { return height; }
		[[nodiscard]] int get_channels() const { return channels; }
		[[nodiscard]] GLuint get_texture() const { return texture_id; }

		std::optional<std::filesystem::path> source_path;
		
	protected:
		GLuint texture_id = 0;

		int display_channels;
		int width = 0;
		int height = 0;
		int channels = 0;
	};

	template <typename Type>
	class TImage final : public IImage
	{
	public:
		explicit TImage(const std::filesystem::path& path)
			: IImage(path)
		{
			Type* raw_data = stbi_load(source_path.value().string().c_str(), &width, &height, &channels, 4);
			display_channels = 4;
			glGenTextures(1, &texture_id);
			set_texture_data(raw_data);

			
			data.resize(4);
			
			for (int c = 0; c < 4; ++c)
			{
				auto& channel = data[c];
				channel.resize(width * height);

				for (int i = 0; i < width * height; ++i) channel[i] = raw_data[i * 4 + c];
			}

			stbi_image_free(raw_data);
		}


		explicit TImage(const int in_with, const int in_height, const int in_channels)
			: IImage(in_with, in_height, in_channels)
		{
			display_channels = in_channels;
			data.resize(4);
			for (int i = 0; i <  4; ++i)
			{
				data[i].resize(width * height);
			}
			glGenTextures(1, &texture_id);
		}

		void set_channel_data(const std::vector<Type>& channel_data, const int channel_offset)
		{
			data[channel_offset] = channel_data;
		}

		void rebuild_texture() {
			set_texture_data(gen_data_from_channels(channels).data());
		}

		~TImage() {
			glDeleteTextures(1, &texture_id);
		}

		[[nodiscard]] Type& get_pixel(const int channel, const int x, const int y)
		{
			return data[channel][x + y * width];
		}

		[[nodiscard]] Type& get_pixel(const int channel, const size_t pixel_index)
		{
			return data[channel][pixel_index];
		}

		[[nodiscard]] std::vector<Type> get_channel_data(const int channel)
		{
			return data[channel];
		}
		
		std::vector<Type> gen_data_from_channels(int desired_channels)
		{
			std::vector<Type> result;
			result.resize(width * height * desired_channels);

			for (int c = 0; c < desired_channels; ++c)
			{
				for (int i = 0; i < width * height; ++i) {
					result[i * desired_channels + c] = data[c][i];
				}
			}
			
			return result;
		}
	
	private:

		void set_texture_data(Type* data)
		{
			glBindTexture(GL_TEXTURE_2D, texture_id);
			
			GLuint channel_mask = display_channels == 1 ? GL_RED : display_channels == 3 ? GL_RGB : GL_RGBA;
			
			if constexpr (std::is_same<Type, float>::value) {
				glTexImage2D(GL_TEXTURE_2D, 0, channel_mask, width, height, 0, channel_mask, GL_FLOAT, data);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, channel_mask, width, height, 0, channel_mask, GL_UNSIGNED_BYTE, data);
			}
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}


		std::vector<std::vector<Type>> data;
	};

	typedef TImage<uint8_t> Image;
	typedef TImage<float> HdrImage;

}
