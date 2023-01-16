#include "main_window.h"

#include <fstream>
#include <imgui_internal.h>
#include <iostream>

#include "gfx.h"
#include "graph.h"
#include "window_interface.h"
#include "imgui_operators.h"

#define CONFIG_FILE "Config.json"
#define GRAPH_PATH "UserGraph"

MainWindow::MainWindow(std::filesystem::path in_user_data_path)
	: user_data_path(std::move(in_user_data_path))
{
	window_saved_width = 800;
	window_saved_height = 600;
	window_saved_pos_x = 0;
	window_saved_pos_y = 0;
	window_saved_pos = false;
	if (std::ifstream config_file(user_data_path / CONFIG_FILE); config_file.is_open())
	{
		try
		{
			window_saved_pos = true;
			const auto json = nlohmann::json::parse(config_file);
			if (json.contains("window_width"))
				window_saved_width = json["window_width"];
			if (json.contains("window_height"))
				window_saved_height = json["window_height"];
			if (json.contains("window_pos_x"))
				window_saved_pos_x = json["window_pos_x"];
			else
				window_saved_pos = false;
			if (json.contains("window_pos_y"))
				window_saved_pos_y = json["window_pos_y"];
			else
				window_saved_pos = false;
			if (json.contains("recent_open"))
				display_left_tab = json["recent_open"];
		}
		catch (const std::exception& e)
		{
			Logger::get().add_persistent_log({
				ELogType::Error, std::string("failed to load json : ") + std::string(e.what())
			});
		}
	}
}

void MainWindow::load_layout()
{
	loaded_graphs.clear();
	if (std::ifstream config_file(user_data_path / CONFIG_FILE); config_file.is_open())
	{
		try
		{
			const auto json = nlohmann::json::parse(config_file);
			std::shared_ptr<Graph> last_selected_graph = nullptr;
			for (const auto& graph : json["loaded_graphs"])
			{
				if (!std::filesystem::exists(graph["path"]))
					continue;

				load_or_create_graph(graph["path"]);
				if (json.contains("selected_graph") && graph["path"] == json["selected_graph"])
					last_selected_graph = selected_graph;
			}
			selected_graph = last_selected_graph;
		}
		catch (const std::exception& e)
		{
			Logger::get().add_persistent_log({ELogType::Error, std::string("failed to load json : ") + e.what()});
		}
	}
}

void MainWindow::save_layout()
{
	const auto config_path = user_data_path / CONFIG_FILE;

	// Create user directory if not exists
	if (!exists(config_path.parent_path()))
		if (!create_directories(config_path.parent_path()))
			Logger::get().add_persistent_log({ELogType::Error, std::string("failed to create config directory")});


	if (std::ofstream config_file(user_data_path / CONFIG_FILE); config_file.is_open())
	{
		nlohmann::json js_graph;

		for (const auto& graph : loaded_graphs)
		{
			js_graph[graph->get_path().string()] = {
				{"path", graph->get_path().string()}
			};
		}

		window_saved_width = Gfx::get().get_window_width();
		window_saved_height = Gfx::get().get_window_height();
		window_saved_pos_x = Gfx::get().get_window_pos_x();
		window_saved_pos_y = Gfx::get().get_window_pos_y();

		const nlohmann::json js = {
			{"window_width", window_saved_width},
			{"window_height", window_saved_height},
			{"window_pos_x", window_saved_pos_x},
			{"window_pos_y", window_saved_pos_y},
			{"loaded_graphs", js_graph},
			{"selected_graph", selected_graph ? selected_graph->get_path().string() : ""},
			{"recent_open", display_left_tab}
		};
		config_file << js;
	}
	save_all();
}

void MainWindow::load_or_create_graph(const std::filesystem::path& graph_path)
{
	for (const auto& graph : loaded_graphs)
		if (graph->get_path() == graph_path)
		{
			selected_graph = graph;
			return;
		}

	loaded_graphs.emplace_back(std::make_shared<Graph>(graph_path));
	selected_graph = loaded_graphs.back();
}

#define BUTTON_SIZE 50
#define FILE_BUTTON_HEIGHT 30
#define LEFT_PANEL_WIDTH 200
#define COLOR(r, g, b, a) ImGui::ColorConvertFloat4ToU32(ImVec4((r) / 255.f, (g) / 255.f, (b) / 255.f, (a) / 255.f))

void MainWindow::display()
{
	ImGui::GetStyle().FrameBorderSize = {0};
	ImGui::GetStyle().ItemSpacing = {0, 0};
	ImGui::GetStyle().WindowPadding = {0, 0};

	ImGui::SetNextWindowPos({0, 0});
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize - ImVec2{0, Logger::get().get_display_height()});
	if (ImGui::Begin("loaded_graphs", nullptr,
	                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		ImGui::BeginGroup();
		// Create new graph
		if (ImGui::Button("+", {BUTTON_SIZE, BUTTON_SIZE}))
		{
			is_creating_file = true;
			want_keyboard_focus = true;
			display_left_tab = true;
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("new project");
			ImGui::EndTooltip();
		}

		// Open existing graph
		if (ImGui::Button("O", {BUTTON_SIZE, BUTTON_SIZE}))
		{
			if (const auto new_file = windows::pick_graph_file(); exists(new_file))
			{
				load_or_create_graph(new_file);
				display_left_tab = true;
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Open project from file (*.spg)");
			ImGui::EndTooltip();
		}

		// Show recent graph
		bool pushed_color = false;
		if (display_left_tab)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, COLOR(100, 100, 100, 80));
			pushed_color = true;
		}
		if (ImGui::Button("R", {BUTTON_SIZE, BUTTON_SIZE}))
			display_left_tab = !display_left_tab;
		if (pushed_color)
			ImGui::PopStyleColor();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Show recent projects");
			ImGui::EndTooltip();
		}

		ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - BUTTON_SIZE});

		// Info
		if (ImGui::Button("I", {BUTTON_SIZE, BUTTON_SIZE}))
			std::cout << "help" << std::endl;
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("System information");
			ImGui::EndTooltip();
		}

		ImGui::EndGroup();

		if (display_left_tab)
		{
			ImGui::SameLine();

			if (ImGui::BeginChild("graph_list", {LEFT_PANEL_WIDTH, ImGui::GetContentRegionAvail().y}))
			{
				ImGui::Dummy(ImVec2{0, 10});
				ImGui::PushStyleColor(ImGuiCol_Text, COLOR(255, 255, 255, 200));
				ImGui::Text("Recent Projects");
				ImGui::PopStyleColor();
				ImGui::Dummy(ImVec2{0, 10});
				ImGui::Separator();

				// Create new project input
				if (is_creating_file)
				{
					ImGui::SetNextItemWidth(LEFT_PANEL_WIDTH);
					if (want_keyboard_focus)
					{
						ImGui::SetKeyboardFocusHere(0);
					}
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {
						                    0, FILE_BUTTON_HEIGHT / 2 - ImGui::CalcTextSize("a").y / 2
					                    });
					if (char buf[256] = {"my_project"}; ImGui::InputText("##new_file_name", buf, sizeof buf,
					                                                     ImGuiInputTextFlags_EnterReturnsTrue))
					{
						if (!get_graph_by_name(buf))
						{
							load_or_create_graph(user_data_path / GRAPH_PATH / (std::string(buf) + ".spg"));
							is_creating_file = false;
						}
					}

					// Cancel
					if (!want_keyboard_focus && ImGui::GetCurrentWindow()->GetID("##new_file_name") !=
						ImGui::GetCurrentContext()->ActiveId)
						is_creating_file = false;
					want_keyboard_focus = false;
					ImGui::PopStyleVar();
				}

				// Display project list
				for (const auto& graph : loaded_graphs)
				{
					if (selected_graph == graph)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, COLOR(100, 100, 100, 20));
						ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetCursorScreenPos(),
						                                          ImGui::GetCursorScreenPos() + ImVec2{
							                                          5, FILE_BUTTON_HEIGHT
						                                          }, COLOR(255, 255, 255, 255));
					}
					else
						ImGui::PushStyleColor(ImGuiCol_Text, COLOR(255, 255, 255, 150));
					if (ImGui::Button(graph->name.c_str(), {ImGui::GetContentRegionAvail().x, FILE_BUTTON_HEIGHT}))
						selected_graph = graph;
					ImGui::PopStyleColor();
				}
			}
			ImGui::EndChild();
		}

		ImGui::SameLine();
		if (ImGui::BeginChild("graph_window"))
		{
			if (selected_graph)
			{
				if (ImGui::Button("Edit\nGraph", ImVec2{60, 60}))
					selected_graph->set_enabled_tool(ESpTool::EditGraph);
				ImGui::SameLine();
				if (ImGui::Button("Edit\nWidget", ImVec2{ 60, 60 }))
					selected_graph->set_enabled_tool(ESpTool::EditWidget);
				ImGui::SameLine();
				if (ImGui::Button("Tool\nWidget", ImVec2{ 60, 60 }))
					selected_graph->set_enabled_tool(ESpTool::RunWidget);

				ImGui::Separator();

				ImGui::PushStyleColor(ImGuiCol_ChildBg, COLOR(0, 0, 0, 70));
				selected_graph->draw();
				ImGui::PopStyleColor();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2{0, ImGui::GetIO().DisplaySize.y - Logger::get().get_display_height()});
	ImGui::SetNextWindowSize(ImVec2{ImGui::GetIO().DisplaySize.x, Logger::get().get_display_height()});
	if (ImGui::Begin("bottom_logs", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(),
		                                          COLOR(0, 120, 200, 255));

		Logger::get().display();
	}
	ImGui::End();
}

void MainWindow::save_all() const
{
	for (const auto& graph : loaded_graphs)
		graph->save_to_file();
}

std::shared_ptr<Graph> MainWindow::get_graph_by_name(const std::string& in_name) const
{
	for (const auto& graph : loaded_graphs)
		if (graph->name == in_name)
			return graph;
	return nullptr;
}

void MainWindow::draw_toolbar(Graph& graph)
{
	if (ImGui::Button("Toggle\nEdit\nMode", {50, 50}))
	{
		will_toggle_summary_mode = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Save\nAll", {50, 50}))
	{
		save_layout();
	}
	ImGui::SameLine();
	if (ImGui::Button("Save", {50, 50}))
	{
		graph.save_to_file();
	}
}
