#include "graph.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include <imgui_internal.h>

#include "gfx.h"
#include "imgui_operators.h"
#include "node.h"

static std::unique_ptr<std::unordered_map<std::string, NodeInfo>> registered_nodes = nullptr;

Graph::Graph(const std::string& in_path) : path(in_path), name(in_path), code_context(std::make_shared<CodeContext>())
{
	load_from_file();
}


uint32_t type_color(EType type, float brightness = 1.f)
{
	switch (type)
	{
	case EType::Float:
		return ImGui::ColorConvertFloat4ToU32({brightness, 0, 0, 1});
	case EType::Float2:
		return ImGui::ColorConvertFloat4ToU32({0, brightness, 0, 1});
	case EType::Float3:
		return ImGui::ColorConvertFloat4ToU32({0, 0, brightness, 1});
	case EType::Float4:
		return ImGui::ColorConvertFloat4ToU32({brightness, brightness, brightness, 1});
	case EType::String:
		return ImGui::ColorConvertFloat4ToU32({brightness, 0, brightness, 1});
	case EType::Undefined:
	default:
		return ImGui::ColorConvertFloat4ToU32({0.3f, 0.3f, 0.3f, 1});
	}
}

void Graph::draw_connection(ImVec2 from, ImVec2 to, EType connection_type) const
{
	const uint32_t color = type_color(connection_type);


	const float delta = max(std::abs(from.x - to.x) * 0.5f, std::abs(from.y - to.y) * 0.5f);
	ImGui::GetWindowDrawList()->AddBezierCurve(from,
	                                           from + ImVec2(delta, 0),
	                                           to + ImVec2(-delta, 0),
	                                           to,
	                                           color, max(4 * zoom, 1.5f));
}

void Graph::draw_pin(const MouseHit& pin_infos, EType type, bool connected, const std::string& name,
                     bool text_left)
{
	const bool out_hover = add_detect_hit(pin_infos);

	ImGui::GetWindowDrawList()->AddCircleFilled(pin_infos.position, (out_hover ? 12.f : 10.f) * zoom,
	                                            type_color(type, connected ? out_hover ? 0.5f : 1.f : 0.5f));

	ImGui::GetWindowDrawList()->AddCircleFilled(pin_infos.position, (out_hover ? 9.f : 7.f) * zoom,
	                                            connected
		                                            ? type_color(type, out_hover ? 1.f : 0.5f)
		                                            : ImGui::ColorConvertFloat4ToU32({0, 0, 0, 1}));

	const float top_zoom = max(1.f, zoom);

	ImGui::GetWindowDrawList()->AddText(
		text_left
			? pin_infos.position - ImVec2{ImGui::CalcTextSize(name.c_str()).x + 15 * zoom, 8 * top_zoom}
			: pin_infos.position + ImVec2{15 * zoom, -8 * top_zoom},
		ImGui::ColorConvertFloat4ToU32({1, 1, 1, 1}), name.c_str());
}

void Graph::add_to_selection(const std::shared_ptr<Node>& node, bool should_bring_to_front)
{
	if (should_bring_to_front)
		bring_to_front(node.get());
	selected_nodes.insert(node.get());
}

void Graph::clear_selection()
{
	selected_nodes.clear();
}

bool Graph::is_selected(Node* node) const
{
	return selected_nodes.contains(node);
}

bool Graph::is_hovered(const Node* node) const
{
	return hovered_node.get() == node;
}

bool Graph::add_detect_hit(const MouseHit& hit)
{
	const ImVec2 distance = {
		std::abs(hit.position.x - ImGui::GetMousePos().x), std::abs(hit.position.y - ImGui::GetMousePos().y)
	};
	if (distance.x < hit.radius.x * zoom && distance.y < hit.radius.y * zoom)
	{
		hits.emplace_back(hit);
		return true;
	}
	return false;
}

ImVec2 Graph::pos_to_screen(const ImVec2& from, const ImDrawList* draw_list) const
{
	const auto dl = draw_list ? draw_list : ImGui::GetWindowDrawList();
	const auto min = dl->GetClipRectMin();
	const auto size = dl->GetClipRectMax() - min;
	return (from + pos) * zoom + size / 2 + min;
}

ImVec2 Graph::pos_to_graph(const ImVec2& from, const ImDrawList* draw_list) const
{
	const auto dl = draw_list ? draw_list : ImGui::GetWindowDrawList();
	const auto min = dl->GetClipRectMin();
	const auto size = dl->GetClipRectMax() - min;
	return (from - size / 2 - min) / zoom - pos;
}

void Graph::remove_node(Node* erased_node)
{
	selected_nodes.erase(selected_nodes.find(erased_node));

	for (const auto& input : erased_node->inputs)
		input->link_to(nullptr);

	for (const auto& output : erased_node->outputs)
		output->break_links();

	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].get() == erased_node)
		{
			nodes.erase(nodes.begin() + i);
			break;
		}
	}
}

void Graph::open_context_menu()
{
	ImGui::OpenPopup(("ContextMenu_" + name).c_str());
	is_in_context_menu = true;
	context_menu_pos = ImGui::GetMousePos();
	focused_search_context = false;
	memset(context_menu_search, 0, sizeof context_menu_search);
}


void Graph::draw()
{
	hits.clear();

	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		while (!selected_nodes.empty())
			remove_node(*selected_nodes.begin());
	}

	// Handle drag
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
	{
		pos = pos + ImGui::GetMouseDragDelta(ImGuiMouseButton_Right) / zoom;
	}

	if (ImGui::BeginChild(name.c_str(), ImGui::GetContentRegionAvail(), true,
	                      ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
	{
		if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && (ImGui::IsKeyPressed(ImGuiKey_C, false) || ImGui::IsKeyPressed(
			ImGuiKey_X, false)))
		{
			nlohmann::json clipboard;

			for (const auto& node : selected_nodes)
			{
				if (node)
					clipboard[std::to_string(node->uuid)] = node->serialize(*this);
			}

			std::stringstream serialized;
			serialized << clipboard;
			Gfx::get().set_clipboard(serialized.str());
			if (ImGui::IsKeyPressed(ImGuiKey_X, false))
				while (!selected_nodes.empty())
					remove_node(*selected_nodes.begin());
		}

		// Handle zoom
		{
			const float zoom_delta = ImGui::GetIO().MouseWheel * zoom * 0.1f;
			zoom = std::clamp(zoom + zoom_delta, 0.01f, 4.f);

			const ImVec2 percents = ((ImGui::GetMousePos() - ImGui::GetWindowPos()) / ImGui::GetWindowSize() - 0.5) * -
				1;
			if (zoom_delta != 0)
				pos += ImGui::GetWindowSize() * zoom_delta / (zoom * zoom) * percents;
		}

		// Update transformations
		hovered_node = nullptr;
		for (const auto& node : nodes)
		{
			node->update_nodes_positions();
			if (ImGui::IsMouseHoveringRect(node->screen_min + ImVec2{node->inputs.empty() ? 0 : 39 * zoom, 0},
			                               node->screen_max - ImVec2{node->outputs.empty() ? 0 : 39 * zoom, 0}))
				hovered_node = node;
		}
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !is_selected(hovered_node.get()))
		{
			if (!ImGui::IsKeyDown(ImGuiKey_ModCtrl))
				clear_selection();
			if (hovered_node)
				add_to_selection(hovered_node);
		}
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hovered_node)
		{
			moving_node = true;
		}
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			no_drag_before_release = true;
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
			no_drag_before_release = false;

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			moving_node = false;

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && !hovered_node && no_drag_before_release)
		{
			if (!ImGui::IsKeyDown(ImGuiKey_ModCtrl))
				clear_selection();
			open_context_menu();
		}
		if (ImGui::IsKeyPressed(ImGuiKey_Space) && !hovered_node && no_drag_before_release)
			open_context_menu();

		// Handle drag
		if (moving_node && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::GetActiveID() !=
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

			pos += offscreen_delta;

			for (auto& c_node : selected_nodes)
			{
				Node* node = c_node;
				node->position += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left) / zoom - offscreen_delta;
				node->update_nodes_positions();
			}
			ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
		}

		if (ImGui::IsKeyPressed(ImGuiKey_F))
		{
			ImVec2 avg;
			for (const auto node : selected_nodes)
				avg = avg - node->position;
			avg = avg / static_cast<float>(selected_nodes.size());

			pos = avg;
		}

		if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_A, true))
		{
			if (selected_nodes.size() == nodes.size())
				clear_selection();
			else
				for (const auto node : nodes)
					add_to_selection(node, false);
		}
		// Draw connections
		for (const auto& node : nodes)
			node->draw_connections(*this);

		// Draw nodes
		for (const auto& node : nodes)
			node->display_internal(*this);

		// Handle mouse hits
		const MouseHit* closest_hit = nullptr;
		float closest_distance = 0;
		for (auto& hit : hits)
		{
			const ImVec2 d2 = {
				std::abs(hit.position.x - ImGui::GetMousePos().x), std::abs(hit.position.y - ImGui::GetMousePos().y)
			};
			const float distance = min(d2.x, d2.y);
			if (distance < closest_distance || ! closest_hit)
			{
				closest_distance = distance;
				closest_hit = &hit;
			}
		}
		if (closest_hit)
		{
			if (closest_hit->node_output || closest_hit->node_input)
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					if (closest_hit->node_input)
						begin_in_out(closest_hit->node_input);
					else if (closest_hit->node_output)
						begin_out_in(closest_hit->node_output);
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
					if (closest_hit->node_input)
						end_out_in(closest_hit->node_input);
					else if (closest_hit->node_output)
						end_in_out(closest_hit->node_output);
			}
		}
		else
		{
			if (!hovered_node && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				selection_rect_start = std::make_shared<ImVec2>(ImGui::GetMousePos() * zoom + pos);
		}
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			selection_rect_start = nullptr;

		if (selection_rect_start)
		{
			const ImVec2 selection_rect_end = ImGui::GetMousePos() * zoom + pos;

			const ImVec2 screen_a = (*selection_rect_start - pos) / zoom;
			const ImVec2 screen_b = (selection_rect_end - pos) / zoom;
			const ImVec2 screen_min = {min(screen_a.x, screen_b.x), min(screen_a.y, screen_b.y)};
			const ImVec2 screen_max = {max(screen_a.x, screen_b.x), max(screen_a.y, screen_b.y)};
			ImGui::GetForegroundDrawList()->AddRectFilled(screen_min, screen_max,
			                                              ImGui::ColorConvertFloat4ToU32({1, 1, 1, 0.2f}));
			ImGui::GetForegroundDrawList()->AddRect(screen_min, screen_max,
			                                        ImGui::ColorConvertFloat4ToU32({1, 1, 1, 1.f}));

			if (!ImGui::IsKeyDown(ImGuiKey_ModCtrl))
				clear_selection();
			for (const auto& node : nodes)
			{
				if (screen_max.x > node->screen_min.x && screen_max.y > node->screen_min.y && screen_min.x < node->
					screen_max.x && screen_min.y < node->screen_max.y)
				{
					add_to_selection(node, false);
				}
			}
		}

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !is_in_context_menu)
		{
			if (out_to_in || in_to_out)
			{
				open_context_menu();
			}
			if (in_to_out && in_to_out->target())
				in_to_out->link_to(nullptr);
		}

		if (out_to_in)
		{
			draw_connection(out_to_in->position, is_in_context_menu ? context_menu_pos : ImGui::GetMousePos(),
			                out_to_in->on_get_type.execute());
		}

		if (in_to_out)
		{
			draw_connection(is_in_context_menu ? context_menu_pos : ImGui::GetMousePos(), in_to_out->position,
			                EType::Undefined);
		}

		const auto window_draw_list = ImGui::GetWindowDrawList();

		if (ImGui::BeginPopup(("ContextMenu_" + name).c_str()))
		{
			display_node_context_menu(window_draw_list);
			ImGui::EndPopup();
		}
		else if (is_in_context_menu)
		{
			in_to_out = nullptr;
			out_to_in = nullptr;
			is_in_context_menu = false;
		}

		logger.display();

		if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_V, false))
		{
			try
			{
				nlohmann::json clipboard = nlohmann::json::parse(Gfx::get().get_clipboard());

				remap_uuid_in_json(clipboard);

				clear_selection();

				std::vector<std::shared_ptr<Node>> added_nodes;
				ImVec2 average_pos = {};
				for (const auto& node_js : clipboard)
				{
					const auto node = spawn_by_name(node_js["type"]);
					if (!node)
					{
						std::cerr << "failed to spawn node of type " << node_js["type"] << std::endl;
						logger.add_persistent_log({
							.type = ELogType::Error,
							.message = std::string("failed to spawn node of type ") + std::string(node_js["type"])
						});
						continue;
					}
					node->deserialize(node_js);
					add_to_selection(node, false);
					added_nodes.emplace_back(node);
					average_pos += node->position;
				}
				average_pos = average_pos / static_cast<float>(added_nodes.size());
				for (const auto& node_js : clipboard)
				{
					const auto link_target = find_node(node_js["uuid"]);
					if (!link_target)
						continue;
					for (const auto& connection : node_js["inputs"])
					{
						if (!connection.contains("from") || !connection.contains("to") || !connection.contains("uuid"))
							continue;
						const auto link_source = find_node(connection["uuid"]);
						if (!link_source)
							continue;

						const auto connection_source = link_source->output_by_name(connection["from"]);
						const auto connection_target = link_target->input_by_name(connection["to"]);

						if (!connection_source || !connection_target)
							continue;

						connection_target->link_to(connection_source);
					}
				}

				ImVec2 delta = pos_to_graph(ImGui::GetMousePos()) - average_pos;
				for (const auto& node : added_nodes)
					node->position += delta;
			}
			catch (const std::exception& e)
			{
				logger.add_persistent_log({
					.type = ELogType::Error, .message = std::string("Failed to past value : ") + e.what()
				});
			}
		}

		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
	}
	ImGui::EndChild();
}


void Graph::begin_out_in(std::shared_ptr<NodeOutput> start)
{
	if (out_to_in == nullptr)
		out_to_in = start;
}

void Graph::end_out_in(std::shared_ptr<NodeInput> end)
{
	end->link_to(out_to_in);
	out_to_in = nullptr;
}

void Graph::begin_in_out(std::shared_ptr<NodeInput> start)
{
	in_to_out = start;
}

void Graph::end_in_out(std::shared_ptr<NodeOutput> end)
{
	if (in_to_out)
		in_to_out->link_to(end);
	in_to_out = nullptr;
}

void Graph::display_node_context_menu(ImDrawList* window_draw_list)
{
	if (!focused_search_context)
	{
		ImGui::SetKeyboardFocusHere(0);
		focused_search_context = true;
	}
	const bool confirm = ImGui::InputText("##search", context_menu_search, sizeof context_menu_search,
	                                      ImGuiInputTextFlags_EnterReturnsTrue);

	const auto is_same = [](char a, char b)
	{
		if (a == b)
			return true;

		if (a >= 'a' && a <= 'z')
			if (a - 'a' + 'A' == b)
				return true;

		if (a >= 'A' && a <= 'Z')
			if (a - 'A' + 'a' == b)
				return true;

		return false;
	};

	const auto pass_filter = [&](const std::vector<std::string>& alias)
	{
		if (context_menu_search[0] == '\0')
			return true;
		const int64_t search_len = strlen(context_menu_search);
		for (const auto& text : alias)
		{
			int64_t start = 0;
			int64_t end = 0;

			for (int64_t i = 0; i < static_cast<int64_t>(text.size()); ++i)
			{
				if (end - start >= search_len)
					break;
				if (is_same(text[i], context_menu_search[i - start]))
					end = i + 1L;
				else
					start = i + 1;
			}
			if (end - start == search_len)
				return true;
		}
		return false;
	};

	std::vector<const NodeInfo*> nodes;
	nodes.reserve(registered_nodes->size());

	for (const auto& node_type : *registered_nodes)
		nodes.emplace_back(&node_type.second);

	std::sort(nodes.begin(), nodes.end(), [](const NodeInfo* a, const NodeInfo* b)
	{
		const std::string& display_name_a = a->alias.empty() ? a->internal_name : a->alias[0];
		const std::string& display_name_b = b->alias.empty() ? b->internal_name : b->alias[0];
		return std::lexicographical_compare(display_name_a.begin(), display_name_a.end(), display_name_b.begin(),
		                                    display_name_b.end());
	});

	for (const auto& node_type : nodes)
	{
		if (!pass_filter(node_type->alias))
			continue;
		std::string display_name = node_type->alias.empty() ? node_type->internal_name : node_type->alias[0];
		if (ImGui::MenuItem(display_name.c_str()) || confirm || (ImGui::IsKeyPressed(ImGuiKey_Enter) &&
			ImGui::GetFocusID() == ImGui::GetCurrentWindow()->GetID(display_name.c_str())))
		{
			if (const auto node = spawn_by_name(node_type->internal_name))
			{
				node->position = pos_to_graph(context_menu_pos, window_draw_list);
				if (out_to_in && !node->inputs.empty())
					node->inputs[0]->link_to(out_to_in);
				if (in_to_out && !node->outputs.empty())
					in_to_out->link_to(node->outputs[0]);
				add_to_selection(node);
			}
			ImGui::CloseCurrentPopup();
			if (confirm)
				break;
		}
	}
}

void Graph::bring_to_front(const Node* node)
{
	if (nodes.empty())
		return;
	if (nodes.back().get() != node)
	{
		size_t i;
		bool found = false;
		for (i = 0; i < nodes.size(); ++i)
			if (nodes[i].get() == node)
			{
				found = true;
				break;
			}
		if (found)
		{
			nodes.emplace_back(nodes[i]);
			const auto item = nodes.erase(nodes.begin() + i);
		}
	}
}

void Graph::load_from_file()
{
	if (!std::filesystem::exists(path + ".json"))
		return;

	std::ifstream input(path + ".json");
	nlohmann::json js;
	try
	{
		js = nlohmann::json::parse(input);
	}
	catch (const std::exception& e)
	{
		logger.add_persistent_log({
			.type = ELogType::Error, .message = std::string("failed to read graph file: ") + e.what()
		});
		return;
	}

	nodes.clear();
	clear_selection();
	hovered_node = nullptr;
	out_to_in = nullptr;
	in_to_out = nullptr;

	if (js.contains("path"))
		path = js["path"];
	name = js["name"];
	pos.x = js["pos_x"];
	pos.y = js["pos_y"];
	zoom = js["zoom"];

	for (const auto& node_js : js["nodes"])
	{
		const auto node = spawn_by_name(node_js["type"]);
		if (!node)
		{
			std::cerr << "failed to spawn node of type " << node_js["type"] << std::endl;
			continue;
		}
		node->deserialize(node_js);
	}
	for (const auto& node_js : js["nodes"])
	{
		const auto link_target = find_node(node_js["uuid"]);
		if (!link_target)
			continue;
		for (const auto& connection : node_js["inputs"])
		{
			if (!connection.contains("from") || !connection.contains("to") || !connection.contains("uuid"))
				continue;
			const auto link_source = find_node(connection["uuid"]);
			if (!link_source)
				continue;

			const auto connection_source = link_source->output_by_name(connection["from"]);
			const auto connection_target = link_target->input_by_name(connection["to"]);

			if (!connection_source || !connection_target)
				continue;

			connection_target->link_to(connection_source);
		}
	}
}

void Graph::save_to_file()
{
	std::ofstream output(path + ".json");

	nlohmann::json js_nodes;

	for (const auto& node : nodes)
	{
		js_nodes[node->get_internal_name()] = node->serialize(*this);
	}

	nlohmann::json js = {
		{"pos_x", pos.x},
		{"pos_y", pos.y},
		{"zoom", zoom},
		{"name", name},
		{"path", path},
		{"nodes", js_nodes},
	};

	output << std::setw(4) << js << std::endl;
}

void Graph::remap_uuid_in_json(nlohmann::json& in_json)
{
	std::unordered_map<size_t, size_t> uuid_map;

	for (const auto& node : in_json)
		uuid_map[node["uuid"]] = gen_uuid();
	for (auto& node : in_json)
	{
		if (node.contains("uuid"))
			node["uuid"] = uuid_map[node["uuid"]];
		if (node.contains("inputs"))
			for (auto& input : node["inputs"])
			{
				if (input.contains("uuid") && uuid_map.contains(input["uuid"]))
					input["uuid"] = uuid_map[input["uuid"]];
				else if (input.contains("uuid"))
					input.erase(input.find("uuid"));
			}
	}
}

void Graph::register_node(const NodeInfo& node_infos)
{
	if (!registered_nodes)
		registered_nodes = std::make_unique<std::unordered_map<std::string, NodeInfo>>();
	(*registered_nodes)[node_infos.internal_name] = node_infos;
}

std::shared_ptr<Node> Graph::spawn_by_name(const std::string& type_name)
{
	const auto found = registered_nodes->find(type_name);
	if (found != registered_nodes->end())
	{
		const auto node = found->second.constructor();
		node->internal_init(gen_uuid());
		node->name = found->second.alias[0];
		node->type_name = found->first;
		node->owning_graph = this;
		node->register_uniform(*code_context);
		nodes.emplace_back(node);
		return node;
	}
	return nullptr;
}

std::shared_ptr<Node> Graph::find_node(int64_t uuid) const
{
	if (uuid < 0)
		return nullptr;

	for (const auto& node : nodes)
	{
		if (node->uuid == uuid)
			return node;
	}
	return nullptr;
}
