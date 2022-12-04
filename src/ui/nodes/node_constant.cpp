#include "node_constant.h"

NodeFloat::NodeFloat()
	: Node("Float"), value(0)
{
	const auto out = add_output("value"); 
	out->on_get_type.add_lambda([]() { return EType::Float; });

	out->on_get_code.add_lambda([](CodeContext& context)-> std::string
	{
		const auto uname = context.generate_name();
		context.add_uniform(uname, EType::Float);
		return std::format("{} = {};", context.glsl_output_var(EType::Float), uname);
	});
}

void NodeFloat::display()
{
	ImGui::DragFloat("value", &value);
}

nlohmann::json NodeFloat::serialize(Graph& graph)
{
	auto json = Node::serialize(graph);
	json["value"] = value;
	return json;
}

void NodeFloat::deserialize(const nlohmann::json& json)
{
	Node::deserialize(json);
	if (json.contains("value"))
		value = json["value"];
}
