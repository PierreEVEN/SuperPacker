#pragma once
#include <memory>
#include <vector>
#include <imgui.h>
#include <string>
#include <event_manager.h>
#include <nlohmann/json.hpp>

#include "out_shader.h"
#include "packer/code.h"
#include "packer/types.h"


class Texture;
class Node;

// result, pos_x, pos_y, res_x, res_y
DECLARE_DELEGATE_MULTICAST(EventGetValue, float&, float, float, float, float);
DECLARE_DELEGATE_MULTICAST(EventGetRes, float&, float&, float, float, float, float);
DECLARE_DELEGATE_SINGLECAST_RETURN(EventGetOutputType, EType);
DECLARE_DELEGATE_SINGLECAST_RETURN(EventGetCode, std::string, CodeContext&);

class NodeOutput
{
public:
	NodeOutput(Node* in_owner, size_t in_index) : owner(in_owner), index(in_index)
	{
	}

	EventGetOutputType on_get_type;
	EventGetCode on_get_code;

	ImVec2 position;
	std::string name;
	Node* owner = nullptr;
	int64_t index = 0;

	std::string generate_shader_code(CodeContext& code_context)
	{
		return on_get_code.execute(code_context);
	}
};

class NodeInput
{
public:
	std::string name;
	ImVec2 position;
	std::shared_ptr<NodeOutput> input;

	operator bool() const
	{
		return input != nullptr;
	}
};

class Node
{
	friend class Graph;
public:
	Node(const std::string& name);

	virtual nlohmann::json serialize(Graph& graph);
	virtual void deserialize(const nlohmann::json& json);

	virtual bool display_internal(Graph& graph);
	virtual void display() = 0;
	void draw_connections(const Graph& graph) const;

	std::shared_ptr<NodeInput> add_input(std::string name);
	std::shared_ptr<NodeOutput> add_output(std::string name);

	[[nodiscard]] std::string get_internal_name() const { return name + "_" + std::to_string(uuid); }

	OutShader& get_display_shader();

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
	OutShader display_shader;
};
