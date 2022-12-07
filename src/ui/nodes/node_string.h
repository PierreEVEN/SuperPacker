#pragma once
#include "ui/node.h"


class Graph;

class TextInput : public Node
{
public:
	TextInput();
	void display() override;

	virtual nlohmann::json serialize(Graph& graph) override;
	void deserialize(const nlohmann::json& json) override;
	
	ESummaryMode summary_mode() const override { return ESummaryMode::Input; }
private:
	std::string value;
};

class DirectoryInput : public Node
{
public:
	DirectoryInput();
	void display() override;

	virtual nlohmann::json serialize(Graph& graph) override;
	void deserialize(const nlohmann::json& json) override;

private:
	std::string value;
};

class AppendText : public Node
{
public:
	AppendText();
	void display() override;

private:

	std::string value;

	std::shared_ptr<NodeInput> a;
	std::shared_ptr<NodeInput> b;
	std::shared_ptr<NodeOutput> out;
};