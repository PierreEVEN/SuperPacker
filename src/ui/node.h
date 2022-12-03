#pragma once
#include <memory>
#include <vector>
#include <imgui.h>
#include <string>
#include <event_manager.h>
#include <nlohmann/json.hpp>


class Texture;
class Node;

// result, pos_x, pos_y, res_x, res_y
DECLARE_DELEGATE_MULTICAST(EventGetValue, float&, float, float, float, float);
DECLARE_DELEGATE_MULTICAST(EventGetRes, float&, float&, float, float, float, float);

class NodeOutput
{
public:
	NodeOutput(Node* in_owner, size_t in_index) : owner(in_owner), index(in_index) {}
	EventGetValue on_get_value;
	EventGetRes on_get_res;
	ImVec2 position;
	std::string name;
	Node* owner = nullptr;
	int64_t index = 0;
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
	Graph(const std::string& in_name);

	void draw();

	template <typename T, typename...Args_T>
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

	ImVec2 pos;
	float zoom = 1;

	void bring_to_front(const Node* node);
	std::shared_ptr<Node> selected_node = nullptr;
	std::shared_ptr<Node> hover_node = nullptr;

	void load_from_file(const std::string& in_name);
	void save_to_file();

	static void register_node(const std::string& type_name, std::function<std::shared_ptr<Node>()> constructor);
	std::shared_ptr<Node> spawn_by_name(const std::string& type_name);

	std::string name;

	[[nodiscard]] std::shared_ptr<Node> find_node(int64_t uuid) const;

private:
	std::shared_ptr<NodeOutput> out_to_in = nullptr;
	std::shared_ptr<NodeInput> in_to_out = nullptr;
	std::vector<std::shared_ptr<Node>> nodes;
};


class Node
{
	friend class Graph;
public:
	Node(const std::string name);

	virtual nlohmann::json serialize(Graph& graph);
	virtual void deserialize(const nlohmann::json json);

	virtual bool display_internal(Graph& graph);
	virtual void display() = 0;
	void draw_connections() const;

	std::shared_ptr<NodeInput> add_input(std::string name);
	std::shared_ptr<NodeOutput> add_output(std::string name);

	[[nodiscard]] std::string get_internal_name() const { return name + "_" + std::to_string(uuid); }

protected:
	std::string type_name;
	std::vector<std::shared_ptr<NodeInput>> inputs;
	std::vector<std::shared_ptr<NodeOutput>> outputs;
	ImVec2 position;
	ImVec2 size;
	int64_t uuid;
	std::string name;
	bool focused = false;
	bool hover = false;
};

class NodeTexture : public Node
{
public:
	NodeTexture();

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
	NodeConstant();
	void display() override;
private:
	float value;
};

class ImageWriteNode : public Node
{
public:
	ImageWriteNode() : Node("Image Write")
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

	void display() override
	{
	}

private:
	std::shared_ptr<NodeInput> color;
	std::shared_ptr<NodeInput> in_x;
	std::shared_ptr<NodeInput> in_y;
};

class NodeAdd : public Node
{
public:
	NodeAdd();

	void display() override
	{
	}

private:
	std::shared_ptr<NodeInput> color;
	std::shared_ptr<NodeInput> value;
};

class NodeMult : public Node
{
public:
	NodeMult();

	void display() override
	{
	}

private:
	std::shared_ptr<NodeInput> color;
	std::shared_ptr<NodeInput> value;
};
