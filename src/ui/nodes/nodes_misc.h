#pragma once

#include "ui/node.h"

class MakeFloat4 : public Node
{
public:
	MakeFloat4();

	void display() override {}
private:
	std::shared_ptr<NodeInput> r;
		std::shared_ptr<NodeInput> g;
	std::shared_ptr<NodeInput> b;
	std::shared_ptr<NodeInput> a;
};

class MakeFloat3 : public Node
{
public:
	MakeFloat3();

	void display() override {}
private:
	std::shared_ptr<NodeInput> r;
	std::shared_ptr<NodeInput> g;
	std::shared_ptr<NodeInput> b;
};

class MakeFloat2 : public Node
{
public:
	MakeFloat2();

	void display() override {}
private:
	std::shared_ptr<NodeInput> r;
	std::shared_ptr<NodeInput> g;
};

class BreakColor : public Node
{
public:
	BreakColor();

	void display() override {}
private:
	std::shared_ptr<NodeInput> in;
};
