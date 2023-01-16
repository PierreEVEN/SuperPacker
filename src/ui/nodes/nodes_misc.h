#pragma once

#include "ui/node.h"

class MakeFloat4 : public Node
{
public:
	MakeFloat4();
	
private:
	std::shared_ptr<InputPin> r;
		std::shared_ptr<InputPin> g;
	std::shared_ptr<InputPin> b;
	std::shared_ptr<InputPin> a;
};

class MakeFloat3 : public Node
{
public:
	MakeFloat3();
	
private:
	std::shared_ptr<InputPin> r;
	std::shared_ptr<InputPin> g;
	std::shared_ptr<InputPin> b;
};

class MakeFloat2 : public Node
{
public:
	MakeFloat2();
	
private:
	std::shared_ptr<InputPin> r;
	std::shared_ptr<InputPin> g;
};

class BreakColor : public Node
{
public:
	BreakColor();
	
private:
	std::shared_ptr<InputPin> in;
};
