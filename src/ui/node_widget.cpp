
#include "node_widget.h"

#include "gfx.h"
#include "main_window.h"
#include "imgui_operators.h"
#include "pin.h"

#define BG_COLOR ImGui::ColorConvertFloat4ToU32({0.3f, 0.3f, 0.35f, 0.65f})
#define BG_COLOR_HOVER ImGui::ColorConvertFloat4ToU32({0.4f, 0.4f, 0.5f, 0.7f})
#define TITLE_BG_COLOR ImGui::ColorConvertFloat4ToU32({0.2f, 0.2f, 0.3f, 0.8f})
#define BORDER_COLOR ImGui::ColorConvertFloat4ToU32({0.1f, 0.1f, 0.1f, 0.5f})

static void custom_draw_callback(const ImDrawList* parent_list, const ImDrawCmd* cmd)
{
	GL_CHECK_ERROR();
	const auto node = static_cast<Node*>(cmd->UserCallbackData);

	if (node->get_display_shader().bind())
	{
		node->get_graph().code_ctx().update_uniforms();

		const int fb_height = static_cast<int>(ImGui::GetIO().DisplaySize.y * ImGui::GetIO().DisplayFramebufferScale.y);

		const auto window_clip_rect = parent_list->_CmdHeader.ClipRect;
		const ImVec2 clip_min(max(cmd->ClipRect.x, window_clip_rect.x), max(cmd->ClipRect.y, window_clip_rect.y));
		const ImVec2 clip_max(min(cmd->ClipRect.z, window_clip_rect.z), min(cmd->ClipRect.w, window_clip_rect.w));

		if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
			return;

		// Apply scissor/clipping rectangle (Y is inverted in OpenGL)
		glScissor(static_cast<int>(clip_min.x),
			static_cast<int>(static_cast<float>(fb_height) - clip_max.y),
			static_cast<int>(clip_max.x - clip_min.x),
			static_cast<int>(clip_max.y - clip_min.y));

		// Draw UVs
		const float normalized_clip_rect[4] = {
			cmd->ClipRect.x / ImGui::GetIO().DisplaySize.x,
			1 - cmd->ClipRect.y / ImGui::GetIO().DisplaySize.y,
			cmd->ClipRect.z / ImGui::GetIO().DisplaySize.x,
			1 - cmd->ClipRect.w / ImGui::GetIO().DisplaySize.y,
		};
		glUniform4fv(1, 1, normalized_clip_rect);
		node->get_display_shader().draw();
	}
	GL_CHECK_ERROR();
}

void NodeWidget::display(Graph* graph, Node* node, ESpTool tool)
{
	ImVec2 screen_min = node->get_transform().screen_min;
	ImVec2 screen_max = node->get_transform().screen_max;

	ImGui::SetWindowFontScale(graph->zoom);

	const bool selected = graph->is_selected(node);
	const bool hovered = graph->is_hovered(node);

	// Draw window at location
	ImGui::SetCursorScreenPos(screen_min - ImVec2{ 2, 2 } *graph->zoom);
	if (ImGui::BeginChild(("node_" + node->uuid_as_string()).c_str(), screen_max - screen_min + ImVec2{ 4, 4 } *graph->zoom,
		false,
		ImGuiWindowFlags_NoBackground))
	{
		ImGui::SetCursorScreenPos(screen_min + ImVec2{ 25 * graph->zoom, 30 * graph->zoom });
		const auto dl = ImGui::GetWindowDrawList();

		// Border
		dl->AddRectFilled(screen_min - ImVec2{ 2, 2 } *graph->zoom, screen_max + ImVec2{ 2, 2 } *graph->zoom,
			selected ? ImGui::ColorConvertFloat4ToU32({ 1, 1, 0, 1 }) : BORDER_COLOR,
			20 * graph->zoom);

		// Draw background
		dl->AddRectFilled(screen_min, screen_max, hovered ? BG_COLOR_HOVER : BG_COLOR, 20 * graph->zoom);

		// Draw background
		dl->PushClipRect(screen_min, ImVec2{ screen_max.x, screen_min.y + 30 * graph->zoom }, true);
		dl->AddRectFilled(screen_min, { screen_max.x, screen_min.y + 60 * graph->zoom }, TITLE_BG_COLOR,
			20 * graph->zoom);
		dl->PopClipRect();

		// Draw content
		if (ImGui::BeginChild((node->get_name() + "_content_" + node->uuid_as_string()).c_str(),
			ImGui::GetContentRegionAvail() - ImVec2{ 30 * graph->zoom, 2 * graph->zoom }, false,
			ImGuiWindowFlags_NoBackground))
		{
			ImGui::SetWindowFontScale(graph->zoom);
			dl->PushClipRect(screen_min + ImVec2{ 25 * graph->zoom, 30 * graph->zoom },
				screen_min + ImVec2{ 25 * graph->zoom, 30 * graph->zoom } +
				ImGui::GetContentRegionAvail() - ImVec2{ 30 * graph->zoom, 2 * graph->zoom }, false);
			dl->AddCallback(custom_draw_callback, node);
			dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
			dl->PopClipRect();
			node->display(tool);
			ImGui::SetWindowFontScale(1);
		}
		ImGui::EndChild();

		const auto title_size = ImGui::CalcTextSize(node->get_name().c_str());
		const auto title_pos = ImVec2{
			(screen_min.x + screen_max.x) / 2 - title_size.x / 2, screen_min.y + 5.f * graph->zoom
		};


		if (graph->is_hovered(node) && ImGui::IsMouseHoveringRect(title_pos, title_pos + title_size))
			ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);

		if (graph->is_selected(node) && ImGui::IsMouseHoveringRect(
			node->is_editing_name ? screen_min : title_pos,
			node->is_editing_name ? ImVec2{ screen_max.x, screen_min.y + 25 * graph->zoom } : title_pos + title_size))
		{
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				node->is_editing_name = true;
		}
		else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			node->is_editing_name = false;

		// Draw pin_name
		if (node->is_editing_name)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, (25 * graph->zoom - title_size.y) / 2 });
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 60 * graph->zoom);
			ImGui::SetCursorScreenPos(screen_min + ImVec2{ 25, 5 } *graph->zoom);
			char name_str[256] = {};
			memcpy(name_str, node->get_name().c_str(), min(node->get_name().size() + 1, sizeof name_str));
			if (ImGui::InputText(std::string("##pin_name" + node->uuid_as_string()).c_str(), name_str, sizeof name_str,
				ImGuiInputTextFlags_EnterReturnsTrue))
				node->is_editing_name = false;
			node->set_name(name_str);
			ImGui::PopStyleVar();
		}
		else
			dl->AddText(title_pos, ImGui::ColorConvertFloat4ToU32({ 1, 1, 1, 1 }), node->get_name().c_str());

		// Draw outputs points
		const ImVec2 dims = screen_max - (screen_min + ImVec2{ 0, 25 * graph->zoom + 10 * graph->zoom });
		{
			const float step = dims.y / node->get_outputs().size();
			for (const auto& output : node->get_outputs())
			{
				graph->draw_pin({
								   .position = output->get_display_pos(),
								   .radius = {25, step / 2 / graph->zoom},
								   .node_output = output
					}, output->get_type(), output->is_connected(), output->get_pin_name(), true);
			}
		}


		// Draw input points
		{
			const float step = dims.y / node->get_inputs().size();
			for (const auto& input : node->get_inputs())
			{
				graph->draw_pin({
								   .position = input->get_display_pos(),
								   .radius = {25, step / 2 / graph->zoom},
								   .node_input = input
					},
					input->target() ? input->target()->get_type() : EType::Undefined,
					input->target() ? true : false,
					input->get_pin_name(), false);
			}
		}
	}
	ImGui::EndChild();

	ImGui::SetWindowFontScale(1);


















}
