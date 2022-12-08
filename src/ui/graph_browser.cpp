#include "graph_browser.h"

#include <fstream>
#include <iostream>

#include "gfx.h"
#include "graph.h"

void GraphBrowser::load_defaults(const std::string& layout_name)
{
	loaded_layout_name = layout_name;
	std::ifstream file(save_path + "/" + layout_name + ".json");
	if (file.is_open())
	{
		try
		{
			const auto json = nlohmann::json::parse(file);
			window_saved_width = json["win_width"];
			window_saved_height = json["win_width"];

		}
		catch (const std::exception& e)
		{
			std::cerr << "failed to load json : " << e.what() << std::endl;
		}
	}
}

void GraphBrowser::load_layout()
{	std::ifstream file(save_path + "/" + loaded_layout_name + ".json");
	if (file.is_open())
	{
		try
		{
			const auto json = nlohmann::json::parse(file);


			for (const auto& graph : json["graphes"])
			{
				new_graph(graph["name"]);
			}

		}
		catch (const std::exception& e)
		{
			std::cerr << "failed to load json : " << e.what() << std::endl;
		}
	}
}

void GraphBrowser::save_layout()
{
	save_all();
	
	std::ofstream file(save_path + "/" + loaded_layout_name + ".json");
	if (file.is_open())
	{
		nlohmann::json js_graph;

		for (const auto& graph : graphes)
		{
			js_graph[graph->name] = {
				{"name", graph->name}
			};
		}
		
		window_saved_width = Gfx::get().get_window_width();
		window_saved_height = Gfx::get().get_window_height();

		nlohmann::json js = {
			{"win_width", window_saved_width},
			{"win_height", window_saved_height},
			{"graphes", js_graph}
		};
		file << js;
	}
}

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
					if (will_toggle_summary_mode)
					{
						graph->toggle_summary_mode();
						will_toggle_summary_mode = false;
					}
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
		will_toggle_summary_mode = true;
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
