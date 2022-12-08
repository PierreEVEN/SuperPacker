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

	int get_saved_window_width() { return window_saved_width;}
	int get_saved_window_height() { return window_saved_height;}

private:

	int window_saved_width = 800;
	int window_saved_height = 600;

	void draw_toolbar(Graph& graph);

	std::vector<std::shared_ptr<Graph>> graphes;
	std::string save_path;
	std::string loaded_layout_name;

	bool will_toggle_summary_mode = false;
};
