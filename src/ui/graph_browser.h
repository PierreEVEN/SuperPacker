#pragma once
#include <memory>
#include <string>
#include <vector>

#include "nodes/node_maths.h"

class Graph;

class GraphManager
{
public:
	GraphManager(std::filesystem::path in_user_data_path = "resources");

	void load_layout();
	void save_layout();

	void load_or_create_graph(const std::filesystem::path& graph_path);

	void display();

	void save_all() const;

	[[nodiscard]] int get_saved_window_width() const { return window_saved_width; }
	[[nodiscard]] int get_saved_window_height() const { return window_saved_height; }

	bool get_saved_window_pos(int& x, int& y) const
	{
		x = window_saved_pos_x;
		y = window_saved_pos_y;
		return window_saved_pos;
	}

	std::shared_ptr<Graph> get_graph_by_name(const std::string& in_name) const;

private:
	int window_saved_width = 800;
	int window_saved_height = 600;
	int window_saved_pos_x = 0;
	int window_saved_pos_y = 0;
	bool window_saved_pos = false;

	void draw_toolbar(Graph& graph);

	std::vector<std::shared_ptr<Graph>> loaded_graphs;
	const std::filesystem::path user_data_path;

	bool will_toggle_summary_mode = false;
	bool display_left_tab = false;

	bool is_creating_file = false;
	bool want_keyboard_focus = false;

	std::shared_ptr<Graph> selected_graph = nullptr;
};
