#include "node_constant.h"

#include <iostream>
#include <gl/gl3w.h>

#include "gfx.h"

NodeFloat::NodeFloat()
{
	const auto out = add_output("value"); 
	out->on_get_type.add_lambda([]() { return EType::Float; });

	out->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		return std::format("{} = {};", context.glsl_output_var(EType::Float), uniform_var->get_name());
	});
}

NodeFloat::~NodeFloat()
{
	uniform_var->unregister();
}

void NodeFloat::display()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, ImGui::GetContentRegionAvail().y / 2 - ImGui::CalcTextSize("a").y / 2 });
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);	
	ImGui::DragFloat("##value", &value, 0.01f);
	ImGui::PopStyleVar();
}

void NodeFloat::register_uniform(CodeContext& ctx)
{
	Node::register_uniform(ctx);
	uniform_var = ctx.add_uniform(EType::Float);
	uniform_var->on_update_value.add_lambda([&](int location)
	{
		GL_CHECK_ERROR();
		glUniform1f(location, value);
		glGetError();
		GL_CHECK_ERROR();
	});
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
