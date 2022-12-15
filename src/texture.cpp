#include <FreeImagePlus.h>

#include "texture.h"

#include <filesystem>
#include <iostream>
#include <gl/gl3w.h>

void Texture::make_gpu_available()
{
	if (is_valid_cpu && !is_valid_gpu)
	{
		glGenTextures(1, &gl_id);

		glBindTexture(GL_TEXTURE_2D, gl_id);

		GLuint gl_type = 0;

		switch (pixel_format)
		{
		case EPixelFormat::UINT8:
			gl_type = GL_UNSIGNED_BYTE;
			break;
		case EPixelFormat::FLOAT32:
			gl_type = GL_FLOAT;
			break;
		case EPixelFormat::UNDEFINED:
		default: ;
		}

		GLuint gl_format = 0;
		switch (channels)
		{
		case 1:
			gl_format = GL_RED;
			break;
		case 2:
			gl_format = GL_RG;
			break;
		case 3:
			gl_format = GL_RGB;
			break;
		case 4:
			gl_format = GL_RGBA;
			break;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format, gl_type, cpu_data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		is_valid_gpu = true;
	}
}

void Texture::load_from_disc(const std::filesystem::path& path)
{
	if (!is_valid_image_file(path))
	{
		Logger::get().add_persistent_log({ELogType::Error, "failed to load texture from path : " + path.string()});
		return;
	}

	uint8_t found_channels = 0;

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
					found_channels = 4;
				else
					found_channels = 3;
			else
				found_channels = 2;
		else
			found_channels = 1;
	else
		found_channels = 0;

	EPixelFormat found_pixel_format = EPixelFormat::UNDEFINED;

	switch (img.getBitsPerPixel() / found_channels / 8)
	{
	case 1:
		found_pixel_format = EPixelFormat::UINT8;
		break;
	case 4:
		found_pixel_format = EPixelFormat::FLOAT32;
		break;
	default:
		Logger::get().add_persistent_log({
			ELogType::Error,
			"failed to find image pixel format :" + std::to_string(img.getBitsPerPixel() / found_channels / 8)
		});
		break;
	}

	set_format(img.getWidth(), img.getHeight(), found_channels, found_pixel_format);
	reset_memory();

	if (has_r)
		set_channel(0, found_pixel_format, r.accessPixels());
	if (has_g)
		set_channel(1, found_pixel_format, g.accessPixels());
	if (has_b)
		set_channel(2, found_pixel_format, b.accessPixels());
	if (has_a)
		set_channel(3, found_pixel_format, a.accessPixels());
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
		is_valid_cpu = true;
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

void Texture::invalidate_data()
{
	if (is_valid_gpu)
	{
		glDeleteTextures(1, &gl_id);
		gl_id = 0;
	}
	if (is_valid_cpu)
	{
		free(cpu_data);
		cpu_data = nullptr;
	}
	is_valid_gpu = false;
	is_valid_cpu = false;
}

void Texture::set_channel(uint8_t c, EPixelFormat pixel_format, void* source_data)
{
	switch (pixel_format)
	{
	case EPixelFormat::UINT8:
		set_channel<uint8_t>(c, source_data);
		break;
	case EPixelFormat::FLOAT32:
		set_channel<float>(c, source_data);
		break;
	case EPixelFormat::UNDEFINED:
		Logger::get().add_persistent_log({ELogType::Error, "set_channel : undefined pixel format"});
		break;
	default:
		Logger::get().add_persistent_log({ELogType::Error, "set_channel : unhandled pixel format"});
	}
}

Texture::Texture()
{
}

Texture::~Texture()
{
	invalidate_data();
}

uint32_t Texture::get_id()
{
	make_gpu_available();
	return gl_id;
}

void Texture::export_to(const std::filesystem::path& path)
{
}
