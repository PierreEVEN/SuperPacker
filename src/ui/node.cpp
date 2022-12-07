#include "node.h"

#include <algorithm>

#include "texture.h"

#include <imgui_internal.h>

#include "gfx.h"
#include "imgui_operators.h"

#include "graph.h"
#include "out_shader.h"

static int64_t node_uuids = 0;

void Node::draw_connections(const Graph& graph) const
{
	for (const auto& input : inputs)
		if (input->target())
		{
			graph.draw_connection(input->target()->position, input->position,
			                      input->target()->on_get_type.execute());
		}
}

std::string NodeOutput::get_code(CodeContext& code_context)
{
	if (!code)
	{
		code = std::make_shared<std::string>(on_get_code.execute(code_context));
	}
	return *code;
}

void NodeOutput::break_links() const
{
	while (linked())
		link_destinations[0]->link_to(nullptr);
}

void NodeInput::link_to(const std::shared_ptr<NodeOutput>& output)
{
	if (link_target != output)
	{
		if (link_target)
		{
			link_target->link_destinations.erase(std::find(link_target->link_destinations.begin(),
			                                               link_target->link_destinations.end(), this));
			link_target->owner().on_update.clear_object(owning_node);
		}

		link_target = output;
		if (link_target)
		{
			link_target->link_destinations.emplace_back(this);
			link_target->owner().on_update.add_object(owning_node, &Node::mark_dirty);
		}
		owning_node->mark_dirty();
	}
}

Node::Node(std::string in_name) : name(std::move(in_name))
{
	uuid = node_uuids++;
	position = {20, 20};
	size = {300, 200};
}


nlohmann::json Node::serialize(Graph& graph)
{
	nlohmann::json inputs_js;
	for (const auto& input : inputs)
	{
		if (input->target())
		{
			inputs_js[input->name] = {
				{"uuid", input->target()->owner().uuid},
				{"from", input->target()->name},
				{"to", input->name}
			};
		}
	}
	return {
		{"type", type_name},
		{"name", name},
		{"uuid", uuid},
		{"x", position.x},
		{"y", position.y},
		{"width", size.x},
		{"height", size.y},
		{"inputs", inputs_js},
		{"summary", display_in_summary}
	};
}

void Node::deserialize(const nlohmann::json& input)
{
	name = input["name"];
	uuid = input["uuid"];
	node_uuids = max(uuid + 1, node_uuids);
	position = ImVec2{input["x"], input["y"]};
	size = ImVec2{input["width"], input["height"]};
	if (input.contains("summary"))
		display_in_summary = input["summary"];
}

#define BG_COLOR ImGui::ColorConvertFloat4ToU32({0.3f, 0.3f, 0.35f, 0.65f})
#define BG_COLOR_HOVER ImGui::ColorConvertFloat4ToU32({0.4f, 0.4f, 0.5f, 0.7f})
#define TITLE_BG_COLOR ImGui::ColorConvertFloat4ToU32({0.2f, 0.2f, 0.3f, 0.8f})
#define BORDER_COLOR ImGui::ColorConvertFloat4ToU32({0.1f, 0.1f, 0.1f, 0.5f})

static void custom_draw_callback(const ImDrawList* parent_list, const ImDrawCmd* cmd)
{
	GL_CHECK_ERROR();
	const auto node = static_cast<Node*>(cmd->UserCallbackData);

	if (node->get_display_shader().bind(cmd->ClipRect))
	{
		node->get_graph().code_ctx().update_uniforms();

		const float normalized_clip_rect[4] = {
			cmd->ClipRect.x / ImGui::GetIO().DisplaySize.x,
			1 - cmd->ClipRect.y / ImGui::GetIO().DisplaySize.y,
			cmd->ClipRect.z / ImGui::GetIO().DisplaySize.x,
			1 - cmd->ClipRect.w / ImGui::GetIO().DisplaySize.y,
		};

		const int fb_height = static_cast<int>(ImGui::GetIO().DisplaySize.y * ImGui::GetIO().DisplayFramebufferScale.y);
		const ImVec2 clip_min(cmd->ClipRect.x, cmd->ClipRect.y);
		const ImVec2 clip_max(cmd->ClipRect.z, cmd->ClipRect.w);
		if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
			return;

		// Apply scissor/clipping rectangle (Y is inverted in OpenGL)
		glScissor(static_cast<int>(clip_min.x),
		          static_cast<int>(static_cast<float>(fb_height) - clip_max.y),
		          static_cast<int>(clip_max.x - clip_min.x),
		          static_cast<int>(clip_max.y - clip_min.y));
		glUniform4fv(1, 1, normalized_clip_rect);
		node->get_display_shader().draw();
	}
	GL_CHECK_ERROR();
}

void Node::display_internal(Graph& graph)
{
	ImGui::SetWindowFontScale(get_graph().zoom);

	const bool selected = get_graph().is_selected(this);
	const bool hovered = get_graph().is_hovered(this);

	// Draw window at location
	ImGui::SetCursorScreenPos(screen_min - ImVec2{2, 2} * graph.zoom);
	if (ImGui::BeginChild((name + std::to_string(uuid)).c_str(), screen_max - screen_min + ImVec2{4, 4} * graph.zoom,
	                      false,
	                      ImGuiWindowFlags_NoBackground))
	{
		ImGui::SetCursorScreenPos(screen_min + ImVec2{25 * graph.zoom, 30 * get_graph().zoom});
		const auto dl = ImGui::GetWindowDrawList();

		// Border
		dl->AddRectFilled(screen_min - ImVec2{2, 2} * graph.zoom, screen_max + ImVec2{2, 2} * graph.zoom,
		                  selected ? ImGui::ColorConvertFloat4ToU32({1, 1, 0, 1}) : BORDER_COLOR,
		                  20 * graph.zoom);

		// Draw background
		dl->AddRectFilled(screen_min, screen_max, hovered ? BG_COLOR_HOVER : BG_COLOR, 20 * graph.zoom);

		// Draw background
		dl->PushClipRect(screen_min, ImVec2{screen_max.x, screen_min.y + 30 * get_graph().zoom}, true);
		dl->AddRectFilled(screen_min, {screen_max.x, screen_min.y + 60 * get_graph().zoom}, TITLE_BG_COLOR,
		                  20 * graph.zoom);
		dl->PopClipRect();

		// Draw content
		if (ImGui::BeginChild((name + "_content_" + std::to_string(uuid)).c_str(),
		                      ImGui::GetContentRegionAvail() - ImVec2{30 * graph.zoom, 2 * graph.zoom}, false,
		                      ImGuiWindowFlags_NoBackground))
		{
			ImGui::SetWindowFontScale(get_graph().zoom);
			dl->PushClipRect(screen_min + ImVec2{25 * graph.zoom, 30 * get_graph().zoom},
			                 screen_min + ImVec2{25 * graph.zoom, 30 * get_graph().zoom} +
			                 ImGui::GetContentRegionAvail() - ImVec2{30 * graph.zoom, 2 * graph.zoom});
			dl->AddCallback(custom_draw_callback, this);
			dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
			dl->PopClipRect();
			display();
			ImGui::SetWindowFontScale(1);
		}
		ImGui::EndChild();

		if (get_graph().is_selected(this))
		{
			edit_name = true;
		}

		// Draw name
		if (edit_name)
		{
			ImGui::SetCursorScreenPos(screen_min + ImVec2{10, 5});
			char name_str[256];
			memset(name_str, 0, sizeof name_str);
			memcpy(name_str, name.c_str(), min(name.size() + 1, sizeof name_str));
			ImGui::InputText(std::string("##name" + std::to_string(uuid)).c_str(), name_str, sizeof name_str);
			name = name_str;
		}
		else
			dl->AddText({
				            (screen_min.x + screen_max.x) / 2 - ImGui::CalcTextSize(name.c_str()).x / 2,
				            screen_min.y + 5.f * get_graph().zoom
			            },
			            ImGui::ColorConvertFloat4ToU32({1, 1, 1, 1}), name.c_str());

		// Draw outputs points
		const ImVec2 dims = screen_max - (screen_min + ImVec2{0, 25 * get_graph().zoom + 10 * get_graph().zoom});
		{
			const float step = dims.y / outputs.size();
			for (const auto& output : outputs)
			{
				graph.draw_pin({
					               .position = output->position,
					               .radius = {25, step / 2 / graph.zoom},
					               .node_output = output
				               }, output->get_type(), output->linked(), output->name, true);
			}
		}


		// Draw input points
		{
			const float step = dims.y / inputs.size();
			for (const auto& input : inputs)
			{
				graph.draw_pin({
					               .position = input->position,
					               .radius = {25, step / 2 / graph.zoom},
					               .node_input = input
				               },
				               input->target() ? input->target()->get_type() : EType::Undefined,
				               input->target() ? true : false,
				               input->name, false);
			}
		}

		if (summary_mode() != ESummaryMode::Unavailable)
		{
			ImGui::SetCursorScreenPos(ImVec2{screen_max.x - 25 * graph.zoom, screen_min.y + 5 * graph.zoom});
			ImGui::Checkbox(("##summary_" + std::to_string(uuid)).c_str(), &display_in_summary);
		}
	}
	ImGui::EndChild();

	ImGui::SetWindowFontScale(1);

	if (hovered)
	{
		if (!outputs.empty() && (outputs[0]->on_get_type.execute() == EType::Float || outputs[0]->on_get_type.execute()
			== EType::Float2 || outputs[0]->on_get_type.execute() == EType::Float3 || outputs[0]->on_get_type.execute()
			== EType::Float4))
		{
			ImGui::BeginTooltip();
			ImGui::Text("code :\n%s",
			            graph.code_ctx().generate_full_glsl(outputs[0]->get_code(graph.code_ctx())).c_str());
			ImGui::EndTooltip();
		}
	}
}

std::shared_ptr<NodeInput> Node::add_input(std::string name)
{
	auto input = std::make_shared<NodeInput>(this, name);
	inputs.emplace_back(input);
	return input;
}

std::shared_ptr<NodeOutput> Node::add_output(const std::string name)
{
	auto output = std::make_shared<NodeOutput>(this, name);
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

std::shared_ptr<NodeOutput> Node::output_by_name(const std::string& name) const
{
	for (const auto& output : outputs)
		if (output->name == name)
			return output;
	return nullptr;
}

std::shared_ptr<NodeInput> Node::input_by_name(const std::string& name) const
{
	for (const auto& input : inputs)
		if (input->name == name)
			return input;
	return nullptr;
}

void Node::mark_dirty()
{
	for (const auto& output : outputs)
		output->mark_dirty();
	on_update();
}

float Node::calc_min_height() const
{
	return max(outputs.size(), inputs.size()) * 40.f + 30.f;
}

void Node::update_nodes_positions()
{
	size.y = calc_min_height();

	screen_min = get_graph().pos_to_screen(position);
	screen_max = get_graph().pos_to_screen(position + size);

	const float header_height = 35 * get_graph().zoom;
	const float available_space = screen_max.y - screen_min.y - header_height - 10 * get_graph().zoom;
	const float initial_pos = header_height + available_space / 2;

	const float input_step = available_space / inputs.size();
	float input_pos = initial_pos - input_step * (inputs.size() - 1) / 2;
	for (const auto& input : inputs)
	{
		input->position = screen_min + ImVec2{14 * get_graph().zoom, input_pos};
		input_pos += input_step;
	}

	const float output_step = available_space / outputs.size();
	float output_pos = initial_pos - output_step * (outputs.size() - 1) / 2;
	const ImVec2 dims = screen_max - (screen_min + ImVec2{0, 25 * get_graph().zoom + 10 * get_graph().zoom});
	for (const auto& output : outputs)
	{
		output->position = screen_min + ImVec2{dims.x - 14 * get_graph().zoom, output_pos};
		output_pos += output_step;
	}
}
