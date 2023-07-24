

#include <imgui.h>

#include "graph.h"
#include "graph_widgets.h"
#include "node_widget.h"
#include "imgui_operators.h"

void GraphWidgets::display_run(Graph* graph)
{

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertFloat4ToU32({ 0.2f, 0.2f, 0.2f, 1 }));
	if (ImGui::BeginChild("summary"))
	{
		if (ImGui::BeginChild("input panel", ImGui::GetContentRegionAvail() * ImVec2(0.5f, 1.f)))
		{
			for (const auto& node : graph->nodes)
				NodeWidget::display(graph, &*node, ESpTool::RunWidget);
		}
		ImGui::EndChild();
		ImGui::SameLine();

		ImGui::BeginChild("output panel", ImGui::GetContentRegionAvail());
		for (const auto& node : graph->nodes);
		ImGui::EndChild();
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
}
