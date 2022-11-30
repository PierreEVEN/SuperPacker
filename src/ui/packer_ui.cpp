

#include "packer_ui.h"

#include <imgui.h>

#include "channel.h"
#include "packer/packer.h"

void PackerUI::draw(Packer& packer)
{
	ImGui::Columns(2);
	ImGui::BeginGroup();

	const float channel_height = ImGui::GetContentRegionAvail().y / 4.f;

	draw_channel(packer, channel_height, EChannel::Red);
	draw_channel(packer, channel_height, EChannel::Green);
	draw_channel(packer, channel_height, EChannel::Blue);
	draw_channel(packer, channel_height, EChannel::Alpha);


	ImGui::EndGroup();
	ImGui::NextColumn();

	ImGui::Text("Combine");
	ImGui::Columns(1);

}
