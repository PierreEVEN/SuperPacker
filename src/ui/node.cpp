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
		code = std::make_shared<std::string>(on_get_code.execute(code_context));

	return *code;
}

void NodeInput::link_to(const std::shared_ptr<NodeOutput>& output)
{
	if (link_target != output)
	{
		if (link_target)
		{
			link_target->link_count--;
			link_target->owner().on_update.clear_object(owning_node);
		}

		link_target = output;
		if (link_target)
		{
			link_target->link_count++;
			link_target->owner().on_update.add_object(owning_node, &Node::updated_tree);
		}
		owning_node->on_update();
	}
}

Node::Node(std::string in_name) : name(std::move(in_name))
{
	uuid = node_uuids++;
	position = {20, 20};
	size = {300, 200};

	on_update.add_lambda([&]()
	{
		for (const auto& output : outputs)
			output->mark_dirty();
	});
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
		{"inputs", inputs_js}
	};
}

void Node::deserialize(const nlohmann::json& input)
{
	name = input["name"];
	uuid = input["uuid"];
	node_uuids = max(uuid + 1, node_uuids);
	position = ImVec2{input["x"], input["y"]};
	size = ImVec2{input["width"], input["height"]};
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

bool Node::display_internal(Graph& graph)
{
	const auto half_window = ImGui::GetWindowSize() / 2;

	ImVec2 min = graph.pos * graph.zoom + half_window + ImGui::GetWindowPos() + position * graph.zoom;
	ImVec2 max = graph.pos * graph.zoom + half_window + ImGui::GetWindowPos() + (position + size) * graph.zoom;

	if (!graph.selected_node)
		hover = ImGui::IsMouseHoveringRect(min + ImVec2{inputs.empty() ? 0.f : 39.f * graph.zoom, 0.f},
		                                   max + ImVec2{outputs.empty() ? 0.f : -39.f * graph.zoom, 0.f});

	// Handle drag
	if (graph.selected_node.get() == this && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::GetActiveID() !=
		ImGui::GetFocusID())
	{
		position = position + ImGui::GetMouseDragDelta(ImGuiMouseButton_Left) / graph.zoom;
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
		min = graph.pos * graph.zoom + half_window + ImGui::GetWindowPos() + position * graph.zoom;
		max = graph.pos * graph.zoom + half_window + ImGui::GetWindowPos() + (position + size) * graph.zoom;
	}

	const float top_zoom = max(1.f, graph.zoom);

	// Draw window at location
	ImGui::SetCursorScreenPos(min - ImVec2{2, 2} * graph.zoom);
	if (ImGui::BeginChild((name + std::to_string(uuid)).c_str(), max - min + ImVec2{4, 4} * graph.zoom, false,
	                      ImGuiWindowFlags_NoBackground))
	{
		ImGui::SetCursorScreenPos(min + ImVec2{25 * graph.zoom, 30 * top_zoom});
		const auto dl = ImGui::GetWindowDrawList();
		// Border
		dl->AddRectFilled(min - ImVec2{2, 2} * graph.zoom, max + ImVec2{2, 2} * graph.zoom, BORDER_COLOR,
		                  20 * graph.zoom);

		// Draw background
		dl->AddRectFilled(min, max, hover ? BG_COLOR_HOVER : BG_COLOR, 20 * graph.zoom);

		// Draw background
		dl->PushClipRect(min, ImVec2{max.x, min.y + 30 * top_zoom}, true);
		dl->AddRectFilled(min, {max.x, min.y + 60 * top_zoom}, TITLE_BG_COLOR, 20 * graph.zoom);
		dl->PopClipRect();

		// Draw content
		if (ImGui::BeginChild((name + "_content_" + std::to_string(uuid)).c_str(),
		                      ImGui::GetContentRegionAvail() - ImVec2{30 * graph.zoom, 10 * graph.zoom}, false,
		                      ImGuiWindowFlags_NoBackground))
		{
			dl->PushClipRect(ImGui::GetWindowDrawList()->GetClipRectMin(), ImGui::GetWindowDrawList()->GetClipRectMax());
			dl->AddCallback(custom_draw_callback, this);
			dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
			dl->PopClipRect();
			display();
		}
		ImGui::EndChild();

		// Draw name
		ImGui::SetWindowFontScale(top_zoom);
		dl->AddText({(min.x + max.x) / 2 - ImGui::CalcTextSize(name.c_str()).x / 2, min.y + 5.f * top_zoom},
		            ImGui::ColorConvertFloat4ToU32({1, 1, 1, 1}), name.c_str());

		// Draw outputs points
		const ImVec2 dims = max - (min + ImVec2{0, 25 * top_zoom + 10 * top_zoom});
		{
			const float step = dims.y / outputs.size();
			float pos = dims.y / 2 - (step / 2 * (outputs.size() - 1));
			for (const auto& output : outputs)
			{
				output->position = min + ImVec2{dims.x - 14 * graph.zoom, pos + 25 * top_zoom};
				pos += step;
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
			float pos = dims.y / 2 - (step / 2 * (inputs.size() - 1)) + 25 * top_zoom;
			for (const auto& input : inputs)
			{
				input->position = min + ImVec2{14 * graph.zoom, pos};
				pos += step;
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
		ImGui::SetWindowFontScale(1);
	}
	ImGui::EndChild();


	if (hover)
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

	return hover;
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

void Node::updated_tree()
{
	on_update();
}
