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