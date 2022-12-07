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

	void new_graph(std::string graph_name);

	void display();

	void save_all();

private:

	void draw_toolbar(Graph& graph);

	std::vector<std::shared_ptr<Graph>> graphes;
	std::string save_path;
	bool edit_mode = true;
};
