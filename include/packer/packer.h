#pragma once
#include <array>
#include <string>




enum class EChannel
{
	Red = 0,
	Green = 1,
	Blue = 2,
	Alpha = 3
};

std::string e_channel_name(EChannel channel);
int32_t e_channel_color_int(EChannel channel);

class Channel
{
public:
	bool enabled = true;
};

class Packer
{
public:
	void build();
	std::array<Channel, 4> channels;
};