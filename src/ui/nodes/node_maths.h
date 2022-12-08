#pragma once
#include "ui/node.h"

class NodeAdd : public Node
{
public:
	NodeAdd();

	void display() override {}

private:
	std::shared_ptr<NodeInput> a;
	std::shared_ptr<NodeInput> b;
};

class NodeMult : public Node
{
public:
	NodeMult();

	void display() override {}

private:
	std::shared_ptr<NodeInput> a;
	std::shared_ptr<NodeInput> b;
};
