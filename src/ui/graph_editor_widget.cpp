
#include <imgui.h>
#include <imgui_internal.h>

#include "gfx.h"
#include "graph.h"
#include "graph_widgets.h"
#include "node.h"
#include "imgui_operators.h"
#include "node_widget.h"
#include "pin.h"

void GraphWidgets::display_graph(Graph* graph)
{
	graph->hovered_node = nullptr;
	graph->hits.clear();

	// Delete selection
	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
		graph->delete_selection();

	// Move on graph
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
		graph->pos = graph->pos + ImGui::GetMouseDragDelta(ImGuiMouseButton_Right) / graph->zoom;

	// Handle zoom
	{
		const float zoom_delta = ImGui::GetIO().MouseWheel * graph->zoom * 0.1f;
		graph->zoom = std::clamp(graph->zoom + zoom_delta, 0.01f, 4.f);

		const ImVec2 percents = ((ImGui::GetMousePos() - ImGui::GetWindowPos()) / ImGui::GetWindowSize() - 0.5)
			* -1;
		if (std::fabs(ImGui::GetIO().MouseWheel) > 0.00001f)
			graph->pos += ImGui::GetWindowSize() * zoom_delta / (graph->zoom * graph->zoom) * percents;
	}

	if (ImGui::BeginChild(graph->name.c_str(), ImGui::GetContentRegionAvail(), true,
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
	{
		// Copy or Cut selection
		if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && (ImGui::IsKeyPressed(ImGuiKey_C, false) || ImGui::IsKeyPressed(
			ImGuiKey_X, false)))
		{
			nlohmann::json clipboard;

			for (const auto& node : graph->selected_nodes)
				if (node)
					clipboard[std::to_string(node->get_uuid())] = node->serialize(*graph);

			std::stringstream serialized;
			serialized << clipboard;
			Gfx::get().set_clipboard(serialized.str());

			// Delete if using Cut tool
			if (ImGui::IsKeyPressed(ImGuiKey_X, false))
				graph->delete_selection();
		}

		/**********************************************
		 *
		 *	NOT ALLOWED TO DELETE NODE AFTER THAT
		 *
		 **********************************************/

		// Detect if node is hovered
		for (const auto& node : graph->nodes)
		{
			node->update_nodes_positions();
			if (ImGui::IsMouseHoveringRect(node->get_transform().screen_min + ImVec2{ node->get_inputs().empty() ? 0 : 39 * graph->zoom, 0 },
				node->get_transform().screen_max - ImVec2{ node->get_outputs().empty() ? 0 : 39 * graph->zoom, 0 }))
				graph->hovered_node = node;
		}

		// Select or deselect node by click
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !graph->is_selected(graph->hovered_node.get()))
		{
			if (!ImGui::IsKeyDown(ImGuiKey_ModCtrl))
				graph->clear_selection();
			if (graph->hovered_node)
				graph->add_to_selection(graph->hovered_node);
		}

		// Begin drag node
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && graph->hovered_node)
			graph->moving_node = true;

		// Stop drag node
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			graph->moving_node = false;

		// Handle left click for context menu
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			graph->no_drag_before_release = true;
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
			graph->no_drag_before_release = false;
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && !graph->hovered_node && graph->no_drag_before_release)
		{
			if (!ImGui::IsKeyDown(ImGuiKey_ModCtrl))
				graph->clear_selection();
			graph->open_context_menu();
		}

		// Open context menu with space bar
		if (ImGui::IsKeyPressed(ImGuiKey_Space) && !graph->hovered_node && graph->no_drag_before_release)
			graph->open_context_menu();

		// Handle drag
		if (graph->moving_node && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::GetActiveID() !=
			ImGui::GetFocusID())
		{
			const auto pos_min = ImGui::GetWindowPos();
			const auto pos_max = ImGui::GetWindowPos() + ImGui::GetWindowSize();
			const auto mouse_pos = ImGui::GetMousePos();
			ImVec2 offscreen_delta = {};
			offscreen_delta.x += Gfx::get().get_delta_second() * static_cast<float>(std::pow(
				std::clamp(pos_min.x + 40 - mouse_pos.x, 0.f, 40.f) / 40, 4) * 10000);
			offscreen_delta.y += Gfx::get().get_delta_second() * static_cast<float>(std::pow(
				std::clamp(pos_min.y + 40 - mouse_pos.y, 0.f, 40.f) / 40, 4) * 10000);
			offscreen_delta.x -= Gfx::get().get_delta_second() * static_cast<float>(std::pow(
				std::clamp(40 + mouse_pos.x - pos_max.x, 0.f, 40.f) / 40, 4) * 10000);
			offscreen_delta.y -= Gfx::get().get_delta_second() * static_cast<float>(std::pow(
				std::clamp(40 + mouse_pos.y - pos_max.y, 0.f, 40.f) / 40, 4) * 10000);

			graph->pos += offscreen_delta;

			for (auto& c_node : graph->selected_nodes)
			{
				Node* node = c_node;
				node->update_nodes_positions(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left) / graph->zoom - offscreen_delta);
			}
			ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
		}

		// Focus on selected nodes
		if (ImGui::IsKeyPressed(ImGuiKey_F))
		{
			ImVec2 avg;
			for (const auto node : graph->selected_nodes)
				avg = avg - node->get_transform().position;
			avg = avg / static_cast<float>(graph->selected_nodes.size());

			graph->pos = avg;
		}

		// Select all nodes
		if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_A, true))
		{
			if (graph->selected_nodes.size() == graph->nodes.size())
				graph->clear_selection();
			else
				for (const auto& node : graph->nodes)
					graph->add_to_selection(node, false);
		}

		// Draw connections
		for (const auto& node : graph->nodes)
			node->draw_connections(*graph);

		// Draw nodes
		for (const auto& node : graph->nodes)
			NodeWidget::display(graph, &*node, ESpTool::EditGraph);

		// Determine what is under mouse cursor
		const MouseHit* closest_hit = nullptr;
		float closest_distance = 0;
		for (auto& hit : graph->hits)
		{
			const ImVec2 d2 = {
				std::abs(hit.position.x - ImGui::GetMousePos().x), std::abs(hit.position.y - ImGui::GetMousePos().y)
			};
			const float distance = min(d2.x, d2.y);
			if (distance < closest_distance || !closest_hit)
			{
				closest_distance = distance;
				closest_hit = &hit;
			}
		}
		if (closest_hit)
		{
			if (closest_hit->node_output || closest_hit->node_input)
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					if (closest_hit->node_input) {
						graph->begin_in_out(closest_hit->node_input);
					}
					else {
						if (closest_hit->node_output)
							graph->begin_out_in(closest_hit->node_output);
					}
				}
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
					if (closest_hit->node_input) {
						graph->end_out_in(closest_hit->node_input);
					}
					else if (closest_hit->node_output) {
						graph->end_in_out(closest_hit->node_output);
					}
				}
			}
		}
		else
		{
			if (!graph->hovered_node && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				graph->selection_rect_start = std::make_shared<ImVec2>(ImGui::GetMousePos() * graph->zoom + graph->pos);
		}
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			graph->selection_rect_start = nullptr;

		// Selection rectangle
		if (graph->selection_rect_start)
		{
			const ImVec2 selection_rect_end = ImGui::GetMousePos() * graph->zoom + graph->pos;

			const ImVec2 screen_a = (*graph->selection_rect_start - graph->pos) / graph->zoom;
			const ImVec2 screen_b = (selection_rect_end - graph->pos) / graph->zoom;
			const ImVec2 screen_min = { min(screen_a.x, screen_b.x), min(screen_a.y, screen_b.y) };
			const ImVec2 screen_max = { max(screen_a.x, screen_b.x), max(screen_a.y, screen_b.y) };
			ImGui::GetForegroundDrawList()->AddRectFilled(screen_min, screen_max,
				ImGui::ColorConvertFloat4ToU32({ 1, 1, 1, 0.2f }));
			ImGui::GetForegroundDrawList()->AddRect(screen_min, screen_max,
				ImGui::ColorConvertFloat4ToU32({ 1, 1, 1, 1.f }));

			if (!ImGui::IsKeyDown(ImGuiKey_ModCtrl))
				graph->clear_selection();
			for (const auto& node : graph->nodes)
			{
				if (screen_max.x > node->get_transform().screen_min.x && screen_max.y > node->get_transform().screen_min.y && screen_min.x < node->
					get_transform().screen_max.x && screen_min.y < node->get_transform().screen_max.y)
				{
					graph->add_to_selection(node, false);
				}
			}
		}

		// Open context menu when releasing mouse
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !graph->is_in_context_menu)
		{
			if (graph->out_to_in || graph->in_to_out)
			{
				graph->open_context_menu();
			}
			if (graph->in_to_out && graph->in_to_out->target())
				graph->in_to_out->link_to(nullptr);
		}

		// Draw temp connections
		if (graph->out_to_in)
			graph->draw_connection(graph->out_to_in->get_display_pos(),
				graph->is_in_context_menu ? graph->context_menu_pos : ImGui::GetMousePos(),
				graph->out_to_in->on_get_type.execute());
		if (graph->in_to_out)
			graph->draw_connection(graph->is_in_context_menu ? graph->context_menu_pos : ImGui::GetMousePos(),
				graph->in_to_out->get_display_pos(),
				EType::Undefined);

		// Draw context menu
		const auto window_draw_list = ImGui::GetWindowDrawList();
		if (ImGui::BeginPopup(("ContextMenu_" + graph->name).c_str()))
		{
			graph->display_node_context_menu(window_draw_list);
			ImGui::EndPopup();
		}
		else if (graph->is_in_context_menu)
		{
			graph->in_to_out = nullptr;
			graph->out_to_in = nullptr;
			graph->is_in_context_menu = false;
		}

		// Draw logger
		ImGui::SetCursorPos({ 0, ImGui::GetWindowSize().y - graph->logger.get_display_height() });
		if (ImGui::BeginChild("logger", { ImGui::GetWindowSize().x, graph->logger.get_display_height() }))
			graph->logger.display();
		ImGui::EndChild();
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
	}
	ImGui::EndChild();
	
	// Paste selection
	if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_V, false))
	{
		try
		{
			nlohmann::json clipboard = nlohmann::json::parse(Gfx::get().get_clipboard());

			graph->remap_uuid_in_json(clipboard);

			graph->clear_selection();

			std::vector<std::shared_ptr<Node>> added_nodes;
			ImVec2 average_pos = {};
			for (const auto& node_js : clipboard)
			{
				const auto node = graph->spawn_by_name(node_js["type"]);
				if (!node)
				{
					graph->logger.add_persistent_log({
						ELogType::Error,
						std::string("failed to spawn node of type ") + std::string(node_js["type"])
						});
					continue;
				}
				node->deserialize(node_js);
				graph->add_to_selection(node, false);
				added_nodes.emplace_back(node);
				average_pos += node->get_transform().position;
			}
			average_pos = average_pos / static_cast<float>(added_nodes.size());
			for (const auto& node_js : clipboard)
			{
				const auto link_target = graph->find_node(node_js["uuid"]);
				if (!link_target)
					continue;
				for (const auto& connection : node_js["inputs"])
				{
					if (!connection.contains("from") || !connection.contains("to") || !connection.contains(
						"uuid"))
						continue;
					const auto link_source = graph->find_node(connection["uuid"]);
					if (!link_source)
						continue;

					const auto connection_source = link_source->find_output_by_name(connection["from"]);
					const auto connection_target = link_target->find_input_by_name(connection["to"]);

					if (!connection_source || !connection_target)
						continue;

					connection_target->link_to(connection_source);
				}
			}

			ImVec2 delta = graph->pos_to_graph(ImGui::GetMousePos()) - average_pos;
			for (const auto& node : added_nodes)
				node->update_nodes_positions(delta);
		}
		catch (const std::exception& e)
		{
			graph->logger.add_persistent_log({
				ELogType::Error, std::string("Failed to past value : ") + e.what()
				});
		}
	}
}
