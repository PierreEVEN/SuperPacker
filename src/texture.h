#pragma once
#include <array>
#include <iostream>
#include <memory>
#include <vector>

#include "ui/nodes/node_maths.h"

enum class EPixelFormat
{
	UNDEFINED,
	UINT8,
	FLOAT32,
};

class Texture
{
public:
	static std::shared_ptr<Texture> create()
	{
		return std::shared_ptr<Texture>(new Texture());
	}

	~Texture();

	[[nodiscard]] bool ready() const { return is_valid_cpu; }
	[[nodiscard]] uint32_t get_id();
	[[nodiscard]] int res_x() const { return width; }
	[[nodiscard]] int res_y() const { return height; }

	void make_gpu_available();

	[[nodiscard]] bool valid_on_gpu() const { return is_valid_gpu; }
	[[nodiscard]] bool valid_on_cpu() const { return is_valid_cpu; }

	// NEW
	void load_from_disc(const std::filesystem::path& path);
	void reset_memory();
	void set_format(int new_width, int new_height, uint8_t new_channels, EPixelFormat new_format = EPixelFormat::UINT8);

	void clear();

	static bool is_valid_image_file(const std::filesystem::path& path);
	static uint8_t bits_per_pixel(EPixelFormat format);

	template <typename T>
	void set_pixel(int x, int y, int c, T value)
	{
		if (!is_valid_pixel(x, y, c))
			return;
		static_cast<T*>(cpu_data)[(x + y * width) * channels + c] = value;
	}

	template <typename T>
	void clear_channel(uint8_t channel, T value)
	{
		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x)
				set_pixel<T>(x, y, channel, value);
	}

	template <typename T>
	void clear_channels(T color, T alpha)
	{
		for (uint8_t c = 0; c < 3; ++c)
			clear_channel<T>(c, color);
		clear_channel<T>(3, alpha);
	}

	[[nodiscard]] bool is_valid_pixel(int x, int y, int c) const
	{
		return x >= 0 && x < width && y >= 0 && y < height && c >= 0 && c < channels;
	}

	void set_channel(uint8_t c, EPixelFormat pixel_format, void* source_data);

	template <typename T, typename U>
	void set_channel(uint8_t c, U* source_data)
	{
		if (!is_valid_cpu)
			return;

		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x)
				set_pixel<T>(x, y, c, reinterpret_cast<T*>(source_data)[x + y * width]);
	}

private:
	Texture();
	uint32_t gl_id;

	void export_to(const std::filesystem::path& path);

	// NEW
	int width;
	int height;
	int channels;
	EPixelFormat pixel_format;
	bool is_valid_cpu = false;
	bool is_valid_gpu = false;
	void* cpu_data = nullptr;
};
