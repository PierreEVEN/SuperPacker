#pragma once
#include <cstdint>
#include <filesystem>
#include <stb_image.h>


#include "GL/gl3w.h"

namespace SuperPacker {


	class IImage
	{
	public:
		[[nodiscard]] int get_width() const { return width; }
		[[nodiscard]] int get_height() const { return height; }
		[[nodiscard]] int get_channels() const { return channels; }
		[[nodiscard]] GLuint get_texture() const { return texture_id; }
		
	protected:
		GLuint texture_id = 0;

		int width = 0;
		int height = 0;
		int channels = 0;
	};

	template <typename Type>
	class TImage final : public IImage
	{
	public:
		explicit TImage(const std::filesystem::path& source_path)
		{
			Type* raw_data = stbi_load(source_path.string().c_str(), &width, &height, &channels, 4);

			glGenTextures(1, &texture_id);
			glBindTexture(GL_TEXTURE_2D, texture_id);
			if constexpr (std::is_same<Type, float>::value) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, raw_data);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw_data);				
			}
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
	
	private:
		
		std::vector<std::vector<Type>> data;
	};

	typedef TImage<uint8_t> Image;
	typedef TImage<float> HdrImage;

}
