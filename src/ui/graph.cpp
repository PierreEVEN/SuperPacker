#include "graph.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "gfx.h"
#include "imgui_operators.h"
#include "node.h"

static std::unordered_map<std::string, std::function<std::shared_ptr<Node>()>> registered_nodes;

Graph::Graph(const std::string& in_name) : name(in_name), code_context(std::make_shared<CodeContext>())
{
	load_from_file(in_name);
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
		return ImGui::ColorConvertFloat4ToU32({0.1f, 0.1f, 0.1f, 1});
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
	const auto pin_color = type_color(type, out_hover);
	const auto pin_color_hover = type_color(type, out_hover);

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

bool Graph::add_detect_hit(const MouseHit& hit)
{
	const ImVec2 distance = { std::abs(hit.position.x - ImGui::GetMousePos().x), std::abs(hit.position.y - ImGui::GetMousePos().y) };
	if (distance.x < hit.radius.x * zoom && distance.y < hit.radius.y * zoom)
	{
		hits.emplace_back(hit);
		return true;
	}
	return false;
}


void Graph::draw()
{
	hits.clear();

	// Handle drag
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
	{
		pos = pos + ImGui::GetMouseDragDelta(ImGuiMouseButton_Right) / zoom;
	}

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		selected_node = nullptr;

	ImGui::SetNextWindowPos({0, 0});
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	if (ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("file"))
			{
				if (ImGui::MenuItem("Save"))
				{
					save_to_file();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("new"))
			{
				for (const auto& node_type : registered_nodes)
				{
					if (ImGui::MenuItem(node_type.first.c_str()))
					{
						spawn_by_name(node_type.first);
					}
				}
				ImGui::EndMenu();
			}
			ImGui::Dummy({ImGui::GetContentRegionAvail().x - 10 - ImGui::CalcTextSize(name.c_str()).x, 0});
			ImGui::SameLine();
			ImGui::Text(name.c_str());
			ImGui::EndMenuBar();
		}


		// Handle zoom
		{
			const float zoom_delta = ImGui::GetIO().MouseWheel * zoom * 0.1f;
			zoom = std::clamp(zoom + zoom_delta, 0.1f, 4.f);

			ImVec2 percents = ((ImGui::GetMousePos() - ImGui::GetWindowPos()) / ImGui::GetWindowSize() - 0.5) * -1;
			if (zoom_delta != 0)
				pos += ImGui::GetWindowSize() * zoom_delta / (zoom * zoom) * percents;
		}

		// Draw connections
		for (const auto& node : nodes)
			node->draw_connections(*this);

		// Draw nodes
		for (const auto& node : nodes)
		{
			const bool hover = node->display_internal(*this);
			if (hover)
				hover_node = node;
			if (hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				selected_node = node;
		}

		// Handle hits
		MouseHit* closest_hit = nullptr;
		float closest_distance = 0;
		for (auto& hit : hits)
		{
			const ImVec2 d2 = { std::abs(hit.position.x - ImGui::GetMousePos().x), std::abs(hit.position.y - ImGui::GetMousePos().y) };
			const float distance = min(d2.x, d2.y);
			if (distance < closest_distance ||! closest_hit )
			{
				closest_distance = distance;
				closest_hit = &hit;
			}
		}
		if (closest_hit)
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

		if (selected_node)
			bring_to_front(selected_node.get());

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			out_to_in = nullptr;
			if (in_to_out && in_to_out->target())
				in_to_out->link_to(nullptr);
			in_to_out = nullptr;
		}

		if (out_to_in)
		{
			draw_connection(out_to_in->position, ImGui::GetMousePos(), out_to_in->on_get_type.execute());
		}

		if (in_to_out)
		{
			draw_connection(ImGui::GetMousePos(), in_to_out->position, EType::Undefined);
		}
	}
	ImGui::End();

	ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
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

void Graph::load_from_file(const std::string& in_name)
{
	if (!std::filesystem::exists(in_name + ".json"))
		return;

	std::ifstream input(in_name + ".json");
	nlohmann::json js;
	input >> js;

	nodes.clear();
	selected_node = nullptr;
	hover_node = nullptr;
	out_to_in = nullptr;
	in_to_out = nullptr;

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
	std::ofstream output(name + ".json");

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
		{"nodes", js_nodes},
	};

	output << std::setw(4) << js << std::endl;
}

void Graph::register_node(const std::string& type_name, std::function<std::shared_ptr<Node>()> constructor)
{
	registered_nodes[type_name] = constructor;
}

std::shared_ptr<Node> Graph::spawn_by_name(const std::string& type_name)
{
	const auto found = registered_nodes.find(type_name);
	if (found != registered_nodes.end())
	{
		const auto node = found->second();
		node->type_name = type_name;
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
