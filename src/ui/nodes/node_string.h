#pragma once
#include "ui/node.h"


class Graph;

class TextInput : public Node
{
public:
	TextInput();
	void display(ESpTool tool) override;

	virtual nlohmann::json serialize(Graph& graph) override;
	void deserialize(const nlohmann::json& json) override;
	
	void display_summary() override;
	bool is_parameter() const override { return true; }
private:
	std::string value;
};

class DirectoryInput : public Node
{
public:
	DirectoryInput();
	void display(ESpTool tool) override;

	virtual nlohmann::json serialize(Graph& graph) override;
	void deserialize(const nlohmann::json& json) override;
	
	void display_summary() override;
	bool is_parameter() const override { return true; }
private:
	std::string value;
};

class FileInput : public Node
{
public:
	FileInput();
	void display(ESpTool tool) override;

	virtual nlohmann::json serialize(Graph& graph) override;
	void deserialize(const nlohmann::json& json) override;
	
	void display_summary() override;
	bool is_parameter() const override { return true; }
private:
	std::string value;
};

class AppendText : public Node
{
public:
	AppendText();
	void display(ESpTool tool) override;

private:

	std::string value;

	std::shared_ptr<InputPin> a;
	std::shared_ptr<InputPin> b;
	std::shared_ptr<OutputPin> out;
};