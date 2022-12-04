#pragma once
#include "ui/node.h"


class Graph;

class NodeString : public Node
{
public:
	NodeString();
	void display() override;

	virtual nlohmann::json serialize(Graph& graph) override;
	void deserialize(const nlohmann::json& json) override;

private:
	std::string value;
};

class AppendString : public Node
{
public:
	AppendString();
	void display() override {}

private:
	std::shared_ptr<NodeInput> a;
	std::shared_ptr<NodeInput> b;
};