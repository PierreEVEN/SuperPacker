#include <FreeImagePlus.h>

#include "texture.h"

#include <filesystem>
#include <iostream>
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

void Texture::make_gpu_available()
{
	if (gl_id)
	{
	}
}

void Texture::load_from_disc(const std::filesystem::path& path)
{
	if (!is_valid_image_file(path))
	{
		Logger::get().add_persistent_log({ELogType::Error, "failed to load texture from path : " + path.string()});
		return;
	}

	is_valid = false;
	channels = 4;

	fipImage img;
	if (!img.load(path.string().c_str()))
		return;

	fipImage r;
	const bool has_r = img.getChannel(r, FICC_RED);
	fipImage g;
	const bool has_g = img.getChannel(g, FICC_GREEN);
	fipImage b;
	const bool has_b = img.getChannel(b, FICC_BLUE);
	fipImage a;
	const bool has_a = img.getChannel(a, FICC_ALPHA);

	if (has_r)
		if (has_g)
			if (has_b)
				if (has_a)
					channels = 4;
				else
					channels = 3;
			else
				channels = 2;
		else
			channels = 1;
	else
		channels = 0;

	set_format(img.getWidth(), img.getHeight(), channels, pixel_format);
	reset_memory();

	set_channel<uint8_t>(0, r.accessPixels());
	set_channel<uint8_t>(1, g.accessPixels());
	set_channel<uint8_t>(2, b.accessPixels());
	set_channel<uint8_t>(3, a.accessPixels());

	width = img.getWidth();
	height = img.getHeight();
	channels = img.getColorType() == FIC_RGB ? 3 : FIC_RGBALPHA ? 4 : 0;

	size_t i = 0;
	for (auto& channel : channel_data)
	{
		channel.first = false;
		channel.second = new uint8_t[width * height * 4];
		std::memset(channel.second, i++ == 3 ? 255 : 0, width * height * 4);
	}

	for (size_t i = 0; i < channels; ++i)
		channel_data[i].first = true;


	for (size_t c = 0; c < channels; ++c)
	{
		const size_t cc = c == 0 ? 2 : c == 2 ? 0 : c;
		for (size_t p = 0; p < width * height; ++p)
			channel_data[c].second[p] = img.accessPixels()[p * channels + cc];
	}

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
	is_valid = true;
}

void Texture::reset_memory()
{
	invalidate_data();

	if (width <= 0 || height <= 0 || channels <= 0 || channels > 4)
		return;

	const int pixel_count = width * height * channels;

	switch (pixel_format)
	{
	case EPixelFormat::UINT8:
		cpu_data = new uint8_t[pixel_count];
		clear_channels<uint8_t>(0, 255);
		break;
	case EPixelFormat::FLOAT32:
		cpu_data = new float[pixel_count];
		clear_channels<float>(0.f, 1.f);
		break;
	}

	if (cpu_data)
	{
		is_valid = true;
	}
}

bool Texture::is_valid_image_file(const std::filesystem::path& path)
{
	if (!exists(path))
		return false;

	fipImage image;
	return image.load(path.string().c_str(), FIF_LOAD_NOPIXELS);
}

uint8_t Texture::bits_per_pixel(EPixelFormat format)
{
	switch (format)
	{
	case EPixelFormat::UINT8: return 1;
	case EPixelFormat::FLOAT32: return 4;
	}
	Logger::get().add_persistent_log({ELogType::Error, "unhandled pixel format type"});
	return 0;
}

void Texture::set_format(int new_width, int new_height, uint8_t new_channels, EPixelFormat new_format)
{
	if (width == new_width && height == new_height && channels == new_channels && pixel_format == new_format)
		return;

	pixel_format = new_format;
	width = new_width;
	height = new_height;
	channels = new_channels;
	invalidate_data();
}

Texture::Texture()
{
}

Texture::~Texture()
{
	for (auto& channel : channel_data)
		delete[] channel.second;
}

uint32_t Texture::get_id()
{
	make_gpu_available();
	return gl_id;
}

void Texture::export_to(const std::filesystem::path& path)
{
}
