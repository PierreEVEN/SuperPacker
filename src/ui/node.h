#pragma once
#include <memory>
#include <vector>
#include <imgui.h>
#include <string>
#include <event_manager.h>


class Texture;
class Node;

// result, pos_x, pos_y, res_x, res_y
DECLARE_DELEGATE_MULTICAST(EventGetValue, float&, float, float, float, float);
DECLARE_DELEGATE_MULTICAST(EventGetRes, float&, float&, float, float, float, float);

class NodeOutput
{
public:
	EventGetValue on_get_value;
	EventGetRes on_get_res;
	ImVec2 position;
	std::string name;
};
class NodeInput
{
public:
	std::string name;
	ImVec2 position;
	std::shared_ptr<NodeOutput> input;
};


class Graph
{
public:
	void draw();

	template<typename T, typename...Args_T>
	std::shared_ptr<T> create_node(Args_T&&... args)
	{
		const auto node = std::make_shared<T>(std::forward<Args_T>(args)...);
		nodes.emplace_back(node);
		return node;
	}

	void begin_out_in(std::shared_ptr<NodeOutput> start);
	void end_out_in(std::shared_ptr<NodeInput> end);

	void begin_in_out(std::shared_ptr<NodeInput> start);
	void end_in_out(std::shared_ptr<NodeOutput> end);
private:
	std::shared_ptr<NodeOutput> connection_output_to_input = nullptr;
	std::shared_ptr<NodeInput> connection_input_to_output = nullptr;
	std::vector<std::shared_ptr<Node>> nodes;
};


class Node
{
public:
	Node(const std::string name);

	virtual void display_internal(Graph& graph);
	virtual void display() = 0;

	std::shared_ptr<NodeInput> add_input(std::string name);
	std::shared_ptr<NodeOutput> add_output(std::string name);

protected:
	std::vector<std::shared_ptr<NodeInput>> inputs;
	std::vector<std::shared_ptr<NodeOutput>> outputs;
	ImVec2 position;
	ImVec2 size;
	size_t uuid;
	std::string name;
};

class NodeTexture : public Node
{
public:

	NodeTexture(std::shared_ptr<Texture> in_texture);

	void display() override;
private:
	std::shared_ptr<Texture> texture;
	std::shared_ptr<NodeOutput> r;
	std::shared_ptr<NodeOutput> g;
	std::shared_ptr<NodeOutput> b;
	std::shared_ptr<NodeOutput> a;
};

class NodeConstant : public Node
{
public:
	NodeConstant(float in_value);
	void display() override;
private:
	float value;
};

class ImageWriteNode : public Node
{
public:
	ImageWriteNode() : Node("Image output")
	{
		r = add_input("R");
		g = add_input("G");
		b = add_input("B");
		a = add_input("A");
	}
	void display() override;
private:
	std::shared_ptr<NodeInput> r;
	std::shared_ptr<NodeInput> g;
	std::shared_ptr<NodeInput> b;
	std::shared_ptr<NodeInput> a;
};

class TextureResizeNode : public Node
{
public:
	TextureResizeNode();
	void display() override {}
private:
	std::shared_ptr<NodeInput> color;
	std::shared_ptr<NodeInput> in_x;
	std::shared_ptr<NodeInput> in_y;
};

class NodeAdd : public Node
{
public:
	NodeAdd();
	void display() override {}
private:
	std::shared_ptr<NodeInput> color;
	std::shared_ptr<NodeInput> value;
};

class NodeMult : public Node
{
public:
	NodeMult();
	void display() override {}
private:
	std::shared_ptr<NodeInput> color;
	std::shared_ptr<NodeInput> value;
};
