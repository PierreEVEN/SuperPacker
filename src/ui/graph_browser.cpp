#include "graph_browser.h"

#include "graph.h"

void GraphBrowser::new_graph(std::string graph_name)
{
	graphes.emplace_back(std::make_shared<Graph>(save_path + "/" + graph_name));
}

void GraphBrowser::display()
{
	ImGui::SetNextWindowPos({0, 0});
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	if (ImGui::Begin("graphes", nullptr,
	                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		if (ImGui::BeginTabBar("GraphicSettingsTab"))
		{
			for (const auto& graph : graphes)
			{
				if (ImGui::BeginTabItem(graph->name.c_str()))
				{
					if (ImGui::BeginChild("toolbar", {ImGui::GetContentRegionAvail().x, 50}))
					{
						draw_toolbar(*graph);
					}
					ImGui::EndChild();



					graph->draw();
					
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void GraphBrowser::save_all()
{
	for (const auto& graph : graphes)
		graph->save_to_file();
}

void GraphBrowser::draw_toolbar(Graph& graph)
{
	if (ImGui::Button("Toggle\nEdit\nMode", {50, 50}))
	{
		edit_mode = !edit_mode;
	}
	ImGui::SameLine();
	if (ImGui::Button("Save\nAll", {50, 50}))
	{
		save_all();
	}
	ImGui::SameLine();
	if (ImGui::Button("Save", {50, 50}))
	{
		graph.save_to_file();
	}
}
