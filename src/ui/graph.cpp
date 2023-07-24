#include "graph.h"

#include <algorithm>
#include <fstream>

#include <imgui_internal.h>

#include "gfx.h"
#include "graph_widgets.h"
#include "imgui_operators.h"
#include "node.h"
#include "node_widget.h"
#include "pin.h"

static std::unique_ptr<std::unordered_map<std::string, NodeInfo>> registered_nodes = nullptr;

Graph::Graph(std::filesystem::path in_path) : path(std::move(in_path)), code_context(std::make_shared<CodeContext>())
{
	name = path.filename().string();
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

void Graph::delete_selection()
{
	while (!selected_nodes.empty())
		remove_node(*selected_nodes.begin());
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
	switch (enabled_tool)
	{
	case ESpTool::EditGraph:
		GraphWidgets::display_graph(this);
		break;
	case ESpTool::EditWidget:
		GraphWidgets::display_edit(this);
		break;
	case ESpTool::RunWidget:
		GraphWidgets::display_run(this);
		break;
	default: ;
		logger.add_frame_log(Log{ELogType::Error, "unhandled tool mode"});
	}
}


void Graph::begin_out_in(std::shared_ptr<OutputPin> start)
{
	if (out_to_in == nullptr)
		out_to_in = start;
}

void Graph::end_out_in(std::shared_ptr<InputPin> end)
{
	end->link_to(out_to_in);
	out_to_in = nullptr;
}

void Graph::begin_in_out(std::shared_ptr<InputPin> start)
{
	in_to_out = start;
}

void Graph::end_in_out(std::shared_ptr<OutputPin> end)
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
				node->transform.position = pos_to_graph(context_menu_pos, window_draw_list);
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
	// Ensure file is valid
	if (!exists(path) || !std::filesystem::is_regular_file(path))
		return;

	std::ifstream input(path);
	nlohmann::json js;
	try
	{
		js = nlohmann::json::parse(input);
	}
	catch (const std::exception& e)
	{
		logger.add_persistent_log({
			ELogType::Error, std::string("failed to read graph file: ") + e.what()
		});
		return;
	}

	nodes.clear();
	clear_selection();
	hovered_node = nullptr;
	out_to_in = nullptr;
	in_to_out = nullptr;

	if (js.contains("path"))
		path = std::string(js["path"]);
	if (js.contains("enabled_tool"))
		enabled_tool = js["enabled_tool"];
	if (js.contains("pin_name"))
		name = js["pin_name"];
	if (js.contains("pos_x"))
		pos.x = js["pos_x"];
	if (js.contains("pos_y"))
		pos.y = js["pos_y"];
	if (js.contains("zoom"))
		zoom = js["zoom"];

	for (const auto& node_js : js["nodes"])
	{
		const auto node = spawn_by_name(node_js["type"]);
		if (!node)
		{
			Logger::get().add_persistent_log({
				ELogType::Error, std::string("failed to spawn node of type ") + std::string(node_js["type"])
			});
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

			const auto connection_source = link_source->find_output_by_name(connection["from"]);
			const auto connection_target = link_target->find_input_by_name(connection["to"]);

			if (!connection_source || !connection_target)
				continue;

			connection_target->link_to(connection_source);
		}
	}
}

void Graph::save_to_file()
{
	if (!exists(path.parent_path()))
		if (!create_directories(path.parent_path()))
			Logger::get().add_persistent_log({ELogType::Error, "failed to create directory to save graph to"});

	std::ofstream output(path);

	if (!output.is_open())
		return;

	nlohmann::json js_nodes;

	for (const auto& node : nodes)
	{
		js_nodes[node->get_internal_name()] = node->serialize(*this);
	}

	const nlohmann::json js = {
		{"pos_x", pos.x},
		{"pos_y", pos.y},
		{"zoom", zoom},
		{"pin_name", name},
		{"path", path.string()},
		{"nodes", js_nodes},
		{"enabled_tool", enabled_tool}
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
