#include "packer/packer.h"

#include <imgui.h>

std::string e_channel_name(EChannel channel)
{
	switch (channel)
	{
	case EChannel::Red: return "Red";
	case EChannel::Green: return "Green";
	case EChannel::Blue: return "Blue";
	case EChannel::Alpha: return "Alpha";
	default: return "unknown";
	}
}

int32_t e_channel_color_int(EChannel channel)
{
	switch (channel)
	{
	case EChannel::Red: return ImGui::ColorConvertFloat4ToU32({1, 0, 0, 1});
	case EChannel::Green: return ImGui::ColorConvertFloat4ToU32({0, 1, 0, 1});
	case EChannel::Blue: return ImGui::ColorConvertFloat4ToU32({0, 0, 1, 1});
	case EChannel::Alpha: return ImGui::ColorConvertFloat4ToU32({1, 1, 1, 1});
	default: return ImGui::ColorConvertFloat4ToU32({0, 0, 0, 1});
	}
}

void Packer::build()
{
}
