#pragma once
#include <array>
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

	[[nodiscard]] bool ready() const { return is_valid; }
	[[nodiscard]] uint32_t get_id();
	[[nodiscard]] int res_x() const { return width; }
	[[nodiscard]] int res_y() const { return height; }

	float get_color(uint8_t channel, float pos_x, float pos_y, bool filter = false);

	void make_gpu_available();


	// NEW
	void load_from_disc(const std::filesystem::path& path);
	void reset_memory();
	void set_format(int new_width, int new_height, uint8_t new_channels, EPixelFormat new_format = EPixelFormat::UINT8);

	void invalidate_data()
	{
		is_valid = false;
		if (cpu_data)
		{
			delete[] cpu_data;
			cpu_data = nullptr;
		}
	}

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
		return x >= 0 && x < width&& y >= 0 && y < height&& c >= 0 && c < channels;
	}

private:
	Texture();
	uint32_t gl_id;

	std::array<std::pair<bool, uint8_t*>, 4> channel_data;

	void export_to(const std::filesystem::path& path);

	// NEW
	int width;
	int height;
	int channels;
	EPixelFormat pixel_format;
	bool is_valid = false;
	void* cpu_data = nullptr;
};