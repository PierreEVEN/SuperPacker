#pragma once
#include <functional>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

class CodeContext;
enum class EType;
class Node;
class NodeInput;
class NodeOutput;

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

	void draw_connection(ImVec2 from, ImVec2 to, EType connection_type) const;

	CodeContext& code_ctx() const { return *code_context; }

private:
	std::shared_ptr<CodeContext> code_context;
	std::shared_ptr<NodeOutput> out_to_in = nullptr;
	std::shared_ptr<NodeInput> in_to_out = nullptr;
	std::vector<std::shared_ptr<Node>> nodes;
};

