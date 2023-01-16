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


class Texture;
class Node;

DECLARE_DELEGATE_MULTICAST(EventUpdateNode);

class NodeTransform
{
public:
	ImVec2 screen_min;
	ImVec2 screen_max;
	ImVec2 position;
	ImVec2 size;
private:
};


class Node
{
	friend class Graph;
public:
	virtual nlohmann::json serialize(Graph& graph);
	virtual void deserialize(const nlohmann::json& json);
	
	void display_summary_internal();

	virtual void display(ESpTool tool)
	{
	}

	void draw_connections(const Graph& graph) const;

	std::shared_ptr<InputPin> add_input(std::string name);
	std::shared_ptr<OutputPin> add_output(std::string name);

	[[nodiscard]] std::string get_internal_name() const { return name + "_" + std::to_string(uuid); }

	OutShader& get_display_shader();

	EventUpdateNode on_update;

	void mark_dirty();

	[[nodiscard]] Graph& get_graph() const { return *owning_graph; }
	[[nodiscard]] uint64_t get_uuid() const { return uuid; }
	[[nodiscard]] std::string uuid_as_string() const { return std::to_string(uuid); }
	[[nodiscard]] std::string get_name() const { return name; }
	[[nodiscard]] const NodeTransform& get_transform() const { return transform; }
	[[nodiscard]] const std::vector<std::shared_ptr<OutputPin>>& get_outputs() const { return outputs; }
	[[nodiscard]] const std::vector<std::shared_ptr<InputPin>>& get_inputs() const { return inputs; }
	[[nodiscard]] std::shared_ptr<OutputPin> find_output_by_name(const std::string& name) const;
	[[nodiscard]] std::shared_ptr<InputPin> find_input_by_name(const std::string& name) const;
	
	void set_name(const std::string& new_name) { name = new_name; }
	bool is_editing_name = false;

protected:
	virtual void display_summary()
	{
	}

	[[nodiscard]] float calc_min_height() const;

	virtual void register_uniform(CodeContext& ctx)
	{
	}

	void update_nodes_positions();

	std::string type_name;
	std::vector<std::shared_ptr<InputPin>> inputs;
	std::vector<std::shared_ptr<OutputPin>> outputs;
	int64_t uuid;
	OutShader display_shader;
	Graph* owning_graph = nullptr;

private:
	NodeTransform transform;

	std::string name;

	void internal_init(size_t new_uuid);
};
