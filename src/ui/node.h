#pragma once
#include <memory>
#include <vector>
#include <imgui.h>
#include <string>
#include <event_manager.h>
#include <nlohmann/json.hpp>

#include "graph.h"
#include "out_shader.h"
#include "packer/code.h"
#include "packer/types.h"


class Texture;
class Node;

// result, pos_x, pos_y, res_x, res_y
DECLARE_DELEGATE_SINGLECAST_RETURN(EventGetOutputType, EType);
DECLARE_DELEGATE_SINGLECAST_RETURN(EventGetCode, std::string, CodeContext&);
DECLARE_DELEGATE_MULTICAST(EventUpdateNode);

class NodeOutput
{
	friend class NodeInput;
public:
	NodeOutput(Node* in_owner, std::string in_name) : name(std::move(in_name)),
	                                                  owning_node(in_owner)
	{
	}

	EventGetOutputType on_get_type;
	EventGetCode on_get_code;

	ImVec2 position;
	const std::string name;

	std::string get_code(CodeContext& code_context);
	EType get_type() { return on_get_type.execute(); }
	[[nodiscard]] Node& owner() const { return *owning_node; }
	void mark_dirty() { code = nullptr; }
	bool linked() const { return link_count > 0; }
private:
	std::shared_ptr<std::string> code = nullptr;
	int32_t link_count = 0;
	Node* owning_node;
};

class NodeInput
{
public:
	NodeInput(Node* in_owner, std::string in_name) : name(std::move(in_name)), owning_node(in_owner)
	{
	}

	ImVec2 position;

	const std::string name;

	operator bool() const
	{
		return link_target != nullptr;
	}

	void link_to(const std::shared_ptr<NodeOutput>& output);

	[[nodiscard]] const std::shared_ptr<NodeOutput>& target() const { return link_target; }
	[[nodiscard]] Node& owner() const { return *owning_node; }
private:
	Node* owning_node;
	std::shared_ptr<NodeOutput> link_target;
};

class Node
{
	friend class Graph;
public:
	Node(std::string name);

	virtual nlohmann::json serialize(Graph& graph);
	virtual void deserialize(const nlohmann::json& json);


	virtual bool display_internal(Graph& graph);
	virtual void display() = 0;
	void draw_connections(const Graph& graph) const;

	std::shared_ptr<NodeInput> add_input(std::string name);
	std::shared_ptr<NodeOutput> add_output(std::string name);

	[[nodiscard]] std::string get_internal_name() const { return name + "_" + std::to_string(uuid); }

	OutShader& get_display_shader();


	[[nodiscard]] std::shared_ptr<NodeOutput> output_by_name(const std::string& name) const;
	[[nodiscard]] std::shared_ptr<NodeInput> input_by_name(const std::string& name) const;

	EventUpdateNode on_update;

	void updated_tree();

	Graph& get_graph() const { return *owning_graph; }

protected:
	virtual void register_uniform(CodeContext& ctx)
	{
	}

	std::string type_name;
	std::vector<std::shared_ptr<NodeInput>> inputs;
	std::vector<std::shared_ptr<NodeOutput>> outputs;
	ImVec2 position;
	ImVec2 size;
	int64_t uuid;
	std::string name;
	bool focused = false;
	bool hover = false;
	OutShader display_shader;
	Graph* owning_graph = nullptr;
};
