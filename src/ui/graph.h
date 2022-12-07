#pragma once
#include <functional>
#include <imgui.h>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include "event_manager.h"
#include "logger.h"

class CodeContext;
enum class EType;
class Node;
class NodeInput;
class NodeOutput;

struct MouseHit
{
	ImVec2 position;
	ImVec2 radius;
	std::shared_ptr<NodeInput> node_input;
	std::shared_ptr<NodeOutput> node_output;
	std::shared_ptr<Node> node;
};

class Graph
{
public:
	Graph(const std::string& in_path);

	void draw();

	template <typename T, typename...Args_T>
	std::shared_ptr<T> create_node(Args_T&&... args)
	{
		const auto node = std::shared_ptr<T>(new T(std::forward<Args_T>(args)...));
		nodes.emplace_back(node);
		return node;
	}

	ImVec2 pos = {0, 0};
	float zoom = 1;

	void bring_to_front(const Node* node);

	void load_from_file();
	void save_to_file();

	std::shared_ptr<Node> spawn_by_name(const std::string& type_name);

	std::string name;

	[[nodiscard]] std::shared_ptr<Node> find_node(int64_t uuid) const;

	void draw_connection(ImVec2 from, ImVec2 to, EType connection_type) const;
	void draw_pin(const MouseHit& pin_infos, EType type, bool connected, const std::string& name, bool text_left);

	[[nodiscard]] CodeContext& code_ctx() const { return *code_context; }

	void add_to_selection(const std::shared_ptr<Node>& node, bool should_bring_to_front = true);
	void clear_selection();
	[[nodiscard]] bool is_selected(Node* node) const;
	[[nodiscard]] bool is_hovered(const Node* node) const;

	bool add_detect_hit(const MouseHit& hit);

	[[nodiscard]] ImVec2 pos_to_screen(const ImVec2& from, const ImDrawList* draw_list = nullptr) const;
	[[nodiscard]] ImVec2 pos_to_graph(const ImVec2& from, const ImDrawList* draw_list = nullptr) const;


	void remove_node(Node* erased_node);

	void open_context_menu();

	template <typename T>
	static void register_node()
	{
		register_node(typeid(T).name(), []() { return std::make_shared<T>(); });
	}

	Logger logger;

private:
	std::string path;
	static void register_node(const std::string& type_name, std::function<std::shared_ptr<Node>()> constructor);

	void begin_out_in(std::shared_ptr<NodeOutput> start);
	void end_out_in(std::shared_ptr<NodeInput> end);

	void begin_in_out(std::shared_ptr<NodeInput> start);
	void end_in_out(std::shared_ptr<NodeOutput> end);

	void display_node_context_menu(ImDrawList* window_draw_list);

	std::unordered_set<Node*> selected_nodes;
	std::shared_ptr<Node> hovered_node;
	bool moving_node = false;
	bool no_drag_before_release = true;
	std::vector<MouseHit> hits;

	std::shared_ptr<CodeContext> code_context;
	std::shared_ptr<NodeOutput> out_to_in = nullptr;
	std::shared_ptr<NodeInput> in_to_out = nullptr;
	std::vector<std::shared_ptr<Node>> nodes;
	std::shared_ptr<ImVec2> selection_rect_start;
	bool is_in_context_menu = false;
	bool focused_search_context = false;
	char context_menu_search[256] = {};
	ImVec2 context_menu_pos;
};
