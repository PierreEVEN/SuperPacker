#pragma once

#include "packer/types.h"

class Graph;
class Node;

class NodeWidget
{
public:
	NodeWidget() = delete;
	~NodeWidget() = delete;

	static void display(Graph* graph, Node* node, ESpTool tool);
private:
	static void display_graph(Graph* graph, Node* node);
	static void display_setup(Graph* graph, Node* node);
	static void display_run(Graph* graph, Node* node);
};
