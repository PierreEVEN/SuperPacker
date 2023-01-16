#pragma once
#include "ui/node.h"

class NodeAdd : public Node
{
public:
	NodeAdd();

private:
	std::shared_ptr<InputPin> a;
	std::shared_ptr<InputPin> b;
};

class NodeMult : public Node
{
public:
	NodeMult();

private:
	std::shared_ptr<InputPin> a;
	std::shared_ptr<InputPin> b;
};
