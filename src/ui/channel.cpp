#include "channel.h"

#include <imgui.h>

#include "packer/packer.h"


static ImVec2 operator+(const ImVec2& left, const ImVec2& right)
{
	return {left.x + right.x, left.y + right.y};
}

void draw_channel(Packer& packer, float height, EChannel channel)
{
	if (ImGui::BeginChild(e_channel_name(channel).c_str(), ImVec2(0, height)))
	{
		ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImVec2{2.f, ImGui::GetContentRegionAvail().y}, e_channel_color_int(channel));

		ImGui::Dummy({ 5, 0 });
		ImGui::SameLine();

		ImGui::Checkbox("##enabled", &packer.channels[static_cast<int>(channel)].enabled);
	}
	ImGui::EndChild();
}
