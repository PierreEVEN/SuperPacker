#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "texture.h"

#include <filesystem>
#include <gl/gl3w.h>

float Texture::get_color(uint8_t channel, float pos_x, float pos_y, bool filter)
{
	if (!channel_data[channel].first)
		return -1;

	int pixel_index = static_cast<int>(pos_x + pos_y * width);
	if (pixel_index < 0 || pixel_index >= width * height)
		return -1;

	if (filter)
		return -1; // not handled yet

	return channel_data[channel].second[static_cast<int>(pos_x) + static_cast<int>(pos_y) * width] / 255.f;
}

const std::filesystem::path& Texture::get_path() const
{
	return internal_path;
}

Texture::Texture(const std::filesystem::path& path) : internal_path(path.string())
{
	uint8_t* raw_data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);

	for (auto& channel : channel_data)
	{
		channel.first = false;
		channel.second = new uint8_t[width * height * 4];
		std::memset(channel.second, 0, width * height * 4);
	}

	for (size_t i = 0; i < channels; ++i)
		channel_data[i].first = true;


	for (size_t i = 0; i < width * height; ++i)
		for (size_t c = 0; c < channels; ++c)
			channel_data[c].second[i] = raw_data[i * channels + c];

	glGenTextures(1, &gl_id);

	glBindTexture(GL_TEXTURE_2D, gl_id);

	uint8_t* display_data = new uint8_t[width * height * 4];

	for (size_t i = 0; i < width * height; ++i)
		for (size_t c = 0; c < 4; ++c)
			display_data[i * 4 + c] = channel_data[c].second[i];

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, display_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	delete[] display_data;
	stbi_image_free(raw_data);
}

Texture::~Texture()
{
	for (auto& channel : channel_data)
		delete[] channel.second;
}

void Texture::export_to(const std::filesystem::path& path)
{
}