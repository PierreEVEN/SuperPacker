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
};
