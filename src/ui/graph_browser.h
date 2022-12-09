#pragma once
#include <memory>
#include <string>
#include <vector>

class Graph;

class GraphBrowser
{
public:
	GraphBrowser(const std::string& in_save_path = "resources") : save_path(in_save_path)
	{
	}

	void load_defaults(const std::string& layout_name);
	void load_layout();
	void save_layout();

	void new_graph(std::string graph_name);

	void display();

	void save_all();

	[[nodiscard]] int get_saved_window_width() const { return window_saved_width; }
	[[nodiscard]] int get_saved_window_height() const { return window_saved_height; }

	bool get_saved_window_pos(int& x, int& y) const
	{
		x = window_saved_pos_x;
		y = window_saved_pos_y;
		return window_saved_pos;
	}

private:
	int window_saved_width = 800;
	int window_saved_height = 600;
	int window_saved_pos_x = 0;
	int window_saved_pos_y = 0;
	bool window_saved_pos = false;

	void draw_toolbar(Graph& graph);

	std::vector<std::shared_ptr<Graph>> graphes;
	std::string save_path;
	std::string loaded_layout_name;

	bool will_toggle_summary_mode = false;
};
