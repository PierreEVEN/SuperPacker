#pragma once
#include <array>
#include <memory>
#include <vector>

namespace std
{
	namespace filesystem
	{
		class path;
	}
}

class Texture
{
public:
	static std::shared_ptr<Texture> create(const std::filesystem::path& path)
	{
		return std::shared_ptr<Texture>(new Texture(path));
	}

	~Texture();

	[[nodiscard]] uint32_t get_id() const { return gl_id; }

	float get_color(uint8_t channel, float pos_x, float pos_y, bool filter = false);

	int res_x() const { return width; }
	int res_y() const { return height; }

private:
	Texture(const std::filesystem::path& path);

	int width;
	int height;
	int channels;

	uint32_t gl_id;

	std::array<std::pair<bool, uint8_t*>, 4> channel_data;

	void export_to(const std::filesystem::path& path);
};
