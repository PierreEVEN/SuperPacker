#pragma once
#include <functional>
#include <imgui.h>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include "logger.h"
#include <nlohmann/json.hpp>

#include "packer/types.h"

#define REGISTER_NODE(type, infos) \
struct __node_registerer_##type { \
	__node_registerer_##type() { \
		Graph::register_node<type>(infos); \
	} \
}; \
__node_registerer_##type __instance##type

class CodeContext;
enum class EType;
class Node;
class InputPin;
class OutputPin;

class NodeInfo
{
public:
	NodeInfo() = default;

	NodeInfo(std::string in_category, std::vector<std::string> in_alias) : alias(std::move(in_alias)),
	                                                                       category(std::move(in_category))
	{
	}

	std::string internal_name;
	std::vector<std::string> alias;
	std::string category;
	std::function<std::shared_ptr<Node>()> constructor;
};

struct MouseHit
{
	ImVec2 position;
	ImVec2 radius;
	std::shared_ptr<InputPin> node_input;
	std::shared_ptr<OutputPin> node_output;
	std::shared_ptr<Node> node;
};

class Graph
{
	friend class GraphWidgets;
public:
	Graph(std::filesystem::path in_path);

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

	void delete_selection();
	void remove_node(Node* erased_node);

	void open_context_menu();

	template <typename T>
	static void register_node(NodeInfo node_infos)
	{
		node_infos.constructor = [] { return std::make_shared<T>(); };
		node_infos.internal_name = typeid(T).name();
		register_node(node_infos);
	}

	Logger logger;

	size_t gen_uuid() { return last_generated_id++; }
	void push_uuid(size_t uuid) { last_generated_id = uuid + 1 > last_generated_id ? uuid + 1 : last_generated_id; }

	void set_enabled_tool(ESpTool new_enabled_tool) { enabled_tool = new_enabled_tool; }

	[[nodiscard]] std::filesystem::path get_path() const { return path; }

private:
	void remap_uuid_in_json(nlohmann::json& in_json);

	std::filesystem::path path;
	static void register_node(const NodeInfo& node_infos);

	void begin_out_in(std::shared_ptr<OutputPin> start);
	void end_out_in(std::shared_ptr<InputPin> end);

	void begin_in_out(std::shared_ptr<InputPin> start);
	void end_in_out(std::shared_ptr<OutputPin> end);

	void display_node_context_menu(ImDrawList* window_draw_list);

	std::unordered_set<Node*> selected_nodes;
	std::shared_ptr<Node> hovered_node;
	bool moving_node = false;
	bool no_drag_before_release = true;
	std::vector<MouseHit> hits;

	std::shared_ptr<CodeContext> code_context;
	std::shared_ptr<OutputPin> out_to_in = nullptr;
	std::shared_ptr<InputPin> in_to_out = nullptr;
	std::vector<std::shared_ptr<Node>> nodes;
	std::shared_ptr<ImVec2> selection_rect_start;
	bool is_in_context_menu = false;
	bool focused_search_context = false;
	char context_menu_search[256] = {};
	ImVec2 context_menu_pos;
	size_t last_generated_id = 0;

	ESpTool enabled_tool = ESpTool::EditWidget;
};
