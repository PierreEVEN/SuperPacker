#include "node.h"

#include <algorithm>

#include "texture.h"

#include <imgui_internal.h>

#include "gfx.h"
#include "imgui_operators.h"

#include "graph.h"
#include "out_shader.h"
#include "pin.h"

void Node::draw_connections(const Graph& graph) const
{
	for (const auto& input : inputs)
		if (input->target())
		{
			graph.draw_connection(input->target()->get_display_pos(), input->get_display_pos(),
			                      input->target()->on_get_type.execute());
		}
}

nlohmann::json Node::serialize(Graph& graph)
{
	nlohmann::json inputs_js;
	for (const auto& input : inputs)
	{
		if (input->target())
		{
			inputs_js[input->get_pin_name()] = {
				{"uuid", input->target()->get_parent()->uuid},
				{"from", input->target()->get_pin_name()},
				{"to", input->get_pin_name()}
			};
		}
	}
	return {
		{"type", type_name},
		{"pin_name", name},
		{"uuid", uuid},
		{"x", transform.position.x},
		{"y", transform.position.y},
		{"width", transform.size.x},
		{"height", transform.size.y},
		{"inputs", inputs_js}
	};
}

void Node::deserialize(const nlohmann::json& input)
{
	if (input.contains("pin_name"))
		name = input["pin_name"];
	uuid = input["uuid"];
	get_graph().push_uuid(uuid);
	transform.position = ImVec2{input["x"], input["y"]};
	transform.size = ImVec2{input["width"], input["height"]};
}

void Node::display_summary_internal()
{
	ImGui::SetCursorPosX(20);
	if (ImGui::BeginChild((std::string("summary_##") + std::to_string(uuid)).c_str(),
	                      ImVec2{ImGui::GetContentRegionAvail().x - 40, 200}))
	{
		ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(),
		                                          ImGui::GetWindowPos() + ImGui::GetContentRegionAvail(),
		                                          ImGui::ColorConvertFloat4ToU32({1, 1, 1, 1}), 10);

		ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos() + ImVec2{2.f, 2.f},
		                                          ImGui::GetWindowPos() + ImGui::GetContentRegionAvail() - ImVec2{
			                                          4.f, 4.f
		                                          },
		                                          ImGui::ColorConvertFloat4ToU32({0.3f, 0.3f, 0.3f, 1}), 5);

		ImGui::Text(name.c_str());
		display_summary();
	}
	ImGui::EndChild();
	ImGui::Dummy({0, 20});
}

std::shared_ptr<InputPin> Node::add_input(std::string name)
{
	auto input = std::make_shared<InputPin>(this, name);
	inputs.emplace_back(input);
	return input;
}

std::shared_ptr<OutputPin> Node::add_output(const std::string name)
{
	auto output = std::make_shared<OutputPin>(this, name);
	outputs.emplace_back(output);
	return output;
}

OutShader& Node::get_display_shader()
{
	if (!outputs.empty())
	{
		const auto out_type = outputs[0]->on_get_type.execute();
		if (out_type == EType::Float ||
			out_type == EType::Float2 ||
			out_type == EType::Float3 ||
			out_type == EType::Float4)
			display_shader.set_code(
				owning_graph->code_ctx().generate_full_glsl(outputs[0]->get_code(owning_graph->code_ctx())));
		GL_CHECK_ERROR();
	}
	return display_shader;
}

std::shared_ptr<OutputPin> Node::find_output_by_name(const std::string& name) const
{
	for (const auto& output : outputs)
		if (output->get_pin_name() == name)
			return output;
	return nullptr;
}

std::shared_ptr<InputPin> Node::find_input_by_name(const std::string& name) const
{
	for (const auto& input : inputs)
		if (input->get_pin_name() == name)
			return input;
	return nullptr;
}

void Node::mark_dirty()
{
	for (const auto& output : outputs)
		output->mark_dirty();
	on_update();
}

void Node::internal_init(size_t new_uuid)
{
	uuid = new_uuid;
	transform.position = {20, 20};
	transform.size = {300, 200};
}

float Node::calc_min_height() const
{
	return max(outputs.size(), inputs.size()) * 40.f + 30.f;
}

void Node::update_nodes_positions()
{
	transform.size.y = calc_min_height();

	transform.screen_min = get_graph().pos_to_screen(transform.position);
	transform.screen_max = get_graph().pos_to_screen(transform.position + transform.size);

	const float header_height = 35 * get_graph().zoom;
	const float available_space = transform.screen_max.y - transform.screen_min.y - header_height - 10 * get_graph().zoom;
	const float initial_pos = header_height + available_space / 2;

	const float input_step = available_space / inputs.size();
	float input_pos = initial_pos - input_step * (inputs.size() - 1) / 2;
	for (const auto& input : inputs)
	{
		input->update_display_pos(transform.screen_min + ImVec2{14 * get_graph().zoom, input_pos});
		input_pos += input_step;
	}

	const float output_step = available_space / outputs.size();
	float output_pos = initial_pos - output_step * (outputs.size() - 1) / 2;
	const ImVec2 dims = transform.screen_max - (transform.screen_min + ImVec2{0, 25 * get_graph().zoom + 10 * get_graph().zoom});
	for (const auto& output : outputs)
	{
		output->update_display_pos(transform.screen_min + ImVec2{dims.x - 14 * get_graph().zoom, output_pos});
		output_pos += output_step;
	}
}
