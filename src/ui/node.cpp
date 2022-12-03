#include "node.h"

#include <algorithm>
#include <fstream>

#include "texture.h"

#include <imgui_internal.h>
#include <iostream>

static std::unordered_map<std::string, std::function<std::shared_ptr<Node>()>> registered_nodes;
static int64_t node_uuids = 0;

static ImVec2 operator+(const ImVec2& left, const ImVec2& right)
{
	return {left.x + right.x, left.y + right.y};
}

static ImVec2 operator+(const ImVec2& left, const float& right)
{
	return {left.x + right, left.y + right};
}

static ImVec2 operator-(const ImVec2& left, const ImVec2& right)
{
	return {left.x - right.x, left.y - right.y};
}

static ImVec2 operator-(const ImVec2& left, const float& right)
{
	return {left.x - right, left.y - right};
}

static ImVec2 operator*(const ImVec2& left, const ImVec2& right)
{
	return {left.x * right.x, left.y * right.y};
}


static ImVec2 operator*(const ImVec2& left, const float& right)
{
	return {left.x * right, left.y * right};
}

static ImVec2 operator/(const ImVec2& left, const float& right)
{
	return {left.x / right, left.y / right};
}

static ImVec2 operator/(const ImVec2& left, const ImVec2& right)
{
	return {left.x / right.x, left.y / right.y};
}

static ImVec2& operator+=(ImVec2& left, const ImVec2& right)
{
	left.x += right.x;
	left.y += right.y;
	return left;
}


static void draw_connection(ImVec2 from, ImVec2 to)
{
	const float delta = std::abs(from.x - to.x) * 0.5f;
	ImGui::GetWindowDrawList()->AddBezierCurve(from,
	                                           from + ImVec2(delta, 0),
	                                           to + ImVec2(-delta, 0),
	                                           to,
	                                           ImGui::ColorConvertFloat4ToU32({1, 1, 0, 1}), 2);
}

Graph::Graph(const std::string& in_name) : name(in_name)
{
	load_from_file(in_name);
}

void Graph::draw()
{
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
			node->draw_connections();

		// Draw nodes
		for (const auto& node : nodes)
		{
			const bool hover = node->display_internal(*this);
			if (hover)
				hover_node = node;
			if (hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				selected_node = node;
		}

		if (selected_node)
			bring_to_front(selected_node.get());

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			out_to_in = nullptr;
			if (in_to_out && in_to_out->input)
				in_to_out->input = nullptr;
			in_to_out = nullptr;
		}

		if (out_to_in)
		{
			draw_connection(out_to_in->position, ImGui::GetMousePos());
		}

		if (in_to_out)
		{
			draw_connection(ImGui::GetMousePos(), in_to_out->position);
		}
	}
	ImGui::End();

	ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
}

void Node::draw_connections() const
{
	for (const auto& input : inputs)
		if (input->input)
			draw_connection(input->input->position, input->position);
}

void Graph::begin_out_in(std::shared_ptr<NodeOutput> start)
{
	if (out_to_in == nullptr)
		out_to_in = start;
}

void Graph::end_out_in(std::shared_ptr<NodeInput> end)
{
	end->input = out_to_in;
	out_to_in = nullptr;
}

void Graph::begin_in_out(std::shared_ptr<NodeInput> start)
{
	in_to_out = start;
}

void Graph::end_in_out(std::shared_ptr<NodeOutput> end)
{
	if (in_to_out)
		in_to_out->input = end;
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
		const auto node_right = find_node(node_js["uuid"]);
		if (!node_right)
			continue;
		size_t i = node_right->inputs.size() - 1;
		for (const auto& connection : node_js["inputs"])
		{
			const auto node_left = find_node(connection["uuid"]);
			if (node_left)
				node_right->inputs[i--]->input = node_left->outputs[connection["index"]];
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
		nodes.emplace_back(node);
		return node;;
	}
	return nullptr;
}

std::shared_ptr<Node> Graph::find_node(int64_t uuid) const
{
	if (uuid < 0)
		return nullptr;

	for (const auto& node : nodes) {
		if (node->uuid == uuid)
			return node;
	}
	return nullptr;
}


Node::Node(const std::string in_name) : name(in_name)
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
		inputs_js[input->name] = {
			{"uuid", input->input ? input->input->owner->uuid : -1},
			{"index", input->input ? input->input->index : -1}
		};
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

void Node::deserialize(const nlohmann::json input)
{
	name = input["name"];
	uuid = input["uuid"];
	node_uuids = std::max(uuid + 1, node_uuids);
	position = ImVec2{input["x"], input["y"]};
	size = ImVec2{input["width"], input["height"]};
}

#define BG_COLOR ImGui::ColorConvertFloat4ToU32({0.3f, 0.3f, 0.35f, 1})
#define BG_COLOR_HOVER ImGui::ColorConvertFloat4ToU32({0.4f, 0.4f, 0.5f, 1})
#define TITLE_BG_COLOR ImGui::ColorConvertFloat4ToU32({0.2f, 0.2f, 0.3f, 1})
#define BORDER_COLOR ImGui::ColorConvertFloat4ToU32({0.4f, 0.4f, 0.4f, 1})

bool Node::display_internal(Graph& graph)
{
	const auto half_window = ImGui::GetWindowSize() / 2;


	ImVec2 min = graph.pos * graph.zoom + half_window + ImGui::GetWindowPos() + position * graph.zoom;
	ImVec2 max = graph.pos * graph.zoom + half_window + ImGui::GetWindowPos() + (position + size) * graph.zoom;

	if (!graph.selected_node)
		hover = ImGui::IsMouseHoveringRect(min + ImVec2{inputs.empty() ? 0.f : 20.f, 0.f},
		                                   max + ImVec2{outputs.empty() ? 0.f : -20.f, 0.f});

	// Handle drag
	if (graph.selected_node.get() == this && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::GetActiveID() !=
		ImGui::GetFocusID())
	{
		position = position + ImGui::GetMouseDragDelta(ImGuiMouseButton_Left) / graph.zoom;
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
		min = graph.pos * graph.zoom + half_window + ImGui::GetWindowPos() + position * graph.zoom;
		max = graph.pos * graph.zoom + half_window + ImGui::GetWindowPos() + (position + size) * graph.zoom;
	}

	const float top_zoom = std::max(1.f, graph.zoom);

	// Draw window at location
	ImGui::SetCursorScreenPos(min - ImVec2{2, 2} * graph.zoom);
	if (ImGui::BeginChild((name + std::to_string(uuid)).c_str(), max - min + ImVec2{4, 4} * graph.zoom, false,
	                      ImGuiWindowFlags_NoBackground))
	{
		ImGui::SetCursorScreenPos(min + ImVec2{2 * graph.zoom, 25 * top_zoom});
		const auto dl = ImGui::GetWindowDrawList();
		// Border
		dl->AddRectFilled(min - ImVec2{2, 2} * graph.zoom, max + ImVec2{2, 2} * graph.zoom, BORDER_COLOR,
		                  20 * graph.zoom);

		// Draw background
		dl->AddRectFilled(min, max, hover ? BG_COLOR_HOVER : BG_COLOR, 20 * graph.zoom);

		// Draw background
		dl->PushClipRect(min, ImVec2{max.x, min.y + 25 * top_zoom}, true);
		dl->AddRectFilled(min, {max.x, min.y + 60 * top_zoom}, TITLE_BG_COLOR, 20 * graph.zoom);
		dl->PopClipRect();

		// Draw content
		if (ImGui::BeginChild((name + "_content_" + std::to_string(uuid)).c_str(),
		                      ImGui::GetContentRegionAvail() - ImVec2{20 * graph.zoom, 0}, false,
		                      ImGuiWindowFlags_NoBackground))
		{
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
				output->position = min + ImVec2{dims.x, pos + 25 * top_zoom};
				const bool out_hover = ImGui::IsMouseHoveringRect(output->position - ImVec2{20 * graph.zoom, step / 2},
				                                                  output->position + ImVec2{20 * graph.zoom, step / 2},
				                                                  false);
				ImGui::GetWindowDrawList()->AddCircleFilled(output->position, (out_hover ? 10.f : 4.f) * graph.zoom,
				                                            out_hover
					                                            ? ImGui::ColorConvertFloat4ToU32({1, 1, 0, 1})
					                                            : ImGui::ColorConvertFloat4ToU32({0.5, 0.5, 0, 1}));

				ImGui::GetWindowDrawList()->AddText(
					output->position - ImVec2{
						ImGui::CalcTextSize(output->name.c_str()).x + 10 * graph.zoom, 8 * top_zoom
					},
					ImGui::ColorConvertFloat4ToU32({1, 1, 1, 1}), output->name.c_str());

				pos += step;

				if (out_hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					graph.begin_out_in(output);
				if (out_hover && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
					graph.end_in_out(output);
			}
		}


		// Draw input points
		{
			const float step = dims.y / inputs.size();
			float pos = dims.y / 2 - (step / 2 * (inputs.size() - 1)) + 25 * top_zoom;
			for (const auto& input : inputs)
			{
				input->position = min + ImVec2{0, pos};
				const bool in_hover = ImGui::IsMouseHoveringRect(input->position - ImVec2{20 * graph.zoom, step / 2},
				                                                 input->position + ImVec2{
					                                                 20 * graph.zoom, step / 2
				                                                 }, false);
				ImGui::GetWindowDrawList()->AddCircleFilled(input->position, (in_hover ? 10.f : 5.f) * graph.zoom,
				                                            in_hover
					                                            ? ImGui::ColorConvertFloat4ToU32({1, 1, 0, 1})
					                                            : ImGui::ColorConvertFloat4ToU32({0.5f, 0.5f, 0, 1}));

				ImGui::GetWindowDrawList()->AddText(
					input->position + ImVec2{10 * graph.zoom, -8 * top_zoom},
					ImGui::ColorConvertFloat4ToU32({1, 1, 1, 1}), input->name.c_str());

				pos += step;
				if (in_hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					graph.begin_in_out(input);
				if (in_hover && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
					graph.end_out_in(input);
			}
		}
		ImGui::SetWindowFontScale(1);
	}
	ImGui::EndChild();
	return hover;
}

std::shared_ptr<NodeInput> Node::add_input(std::string name)
{
	auto input = std::make_shared<NodeInput>();
	input->name = name;
	inputs.emplace_back(input);
	return input;
}

std::shared_ptr<NodeOutput> Node::add_output(std::string name)
{
	auto output = std::make_shared<NodeOutput>(this, outputs.size());
	output->name = name;
	outputs.emplace_back(output);
	return output;
}

void NodeTexture::display()
{
	if (texture)
		ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<size_t>(texture->get_id())),
		             ImGui::GetContentRegionAvail());
}

void NodeConstant::display()
{
	ImGui::DragFloat("value", &value);
}

void ImageWriteNode::display()
{
	ImGui::Dummy({20, 0});
	ImGui::SameLine();
	ImGui::BeginGroup();
	if (r->input)
	{
		float value = 0;
		float res_x = 0;
		float res_y = 0;
		r->input->on_get_value.execute(value, 0, 0, 800, 600);
		r->input->on_get_res.execute(res_x, res_y, 0, 0, 800, 600);
		ImGui::Text("R :\n\tvalue : %f\n\tres x = %f\n\tres y = %f", value, res_x, res_y);
	}
	if (g->input)
	{
		float value = 0;
		float res_x = 0;
		float res_y = 0;
		g->input->on_get_value.execute(value, 0, 0, 800, 600);
		g->input->on_get_res.execute(res_x, res_y, 0, 0, 800, 600);
		ImGui::Text("G :\n\tvalue : %f\n\tres x = %f\n\tres y = %f", value, res_x, res_y);
	}
	if (b->input)
	{
		float value = 0;
		float res_x = 0;
		float res_y = 0;
		b->input->on_get_value.execute(value, 0, 0, 800, 600);
		b->input->on_get_res.execute(res_x, res_y, 0, 0, 800, 600);
		ImGui::Text("B :\n\tvalue : %f\n\tres x = %f\n\tres y = %f", value, res_x, res_y);
	}
	if (a->input)
	{
		float value = 0;
		float res_x = 0;
		float res_y = 0;
		a->input->on_get_value.execute(value, 0, 0, 800, 600);
		a->input->on_get_res.execute(res_x, res_y, 0, 0, 800, 600);
		ImGui::Text("A :\n\tvalue : %f\n\tres x = %f\n\tres y = %f", value, res_x, res_y);
	}
	ImGui::EndGroup();
}

NodeTexture::NodeTexture()
	: Node("Texture")
{
	r = add_output("R");
	r->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_value = texture ? texture->get_color(0, pos_x, pos_y) : -1;
		});
	r->on_get_res.add_lambda(
		[&](float& out_x, float& out_y, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_x = texture ? static_cast<float>(texture->res_x()) : -1;
			out_y = texture ? static_cast<float>(texture->res_y()) : -1;
		});

	g = add_output("G");
	g->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_value = texture ? texture->get_color(1, pos_x, pos_y) : -1;
		});
	g->on_get_res.add_lambda(
		[&](float& out_x, float& out_y, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_x = texture ? static_cast<float>(texture->res_x()) : -1;
			out_y = texture ? static_cast<float>(texture->res_y()) : -1;
		});

	b = add_output("B");
	b->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_value = texture ? texture->get_color(2, pos_x, pos_y) : -1;
		});
	b->on_get_res.add_lambda(
		[&](float& out_x, float& out_y, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_x = texture ? static_cast<float>(texture->res_x()) : -1;
			out_y = texture ? static_cast<float>(texture->res_y()) : -1;
		});

	a = add_output("A");
	a->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_value = texture ? texture->get_color(3, pos_x, pos_y) : -1;
		});
	a->on_get_res.add_lambda(
		[&](float& out_x, float& out_y, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_x = texture ? static_cast<float>(texture->res_x()) : -1;
			out_y = texture ? static_cast<float>(texture->res_y()) : -1;
		});
}

NodeConstant::NodeConstant()
	: Node("Constant")
{
	auto out = add_output("value");
	out->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_value = value;
		});

	out->on_get_res.add_lambda(
		[&](float& out_x, float& out_y, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_x = -1;
			out_y = -1;
		});
}

TextureResizeNode::TextureResizeNode() : Node("Resize")
{
	color = add_input("channel");
	in_x = add_input("size_x");
	in_y = add_input("size_y");
	auto out = add_output("result");
	out->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			if (!in_x->input || !in_y->input || !color->input)
			{
				out_value = -1;
				return;
			}

			float in_value = -1;
			color->input->on_get_value.execute(in_value, pos_x, pos_y, res_x, res_y);
			out_value = in_value;
		});

	out->on_get_res.add_lambda(
		[&](float& res_x, float& res_y, float pos_x, float pos_y, float out_res_x, float out_res_y)
		{
			if (!in_x->input || !in_y->input)
			{
				res_x = -1;
				res_y = -1;
			}

			if (in_x->input)
			{
				float value = -1;
				in_x->input->on_get_value.execute(value, pos_x, pos_y, out_res_x, out_res_y);
				res_x = value;
			}

			if (in_y->input)
			{
				float value = -1;
				in_y->input->on_get_value.execute(value, pos_x, pos_y, out_res_x, out_res_y);
				res_y = value;
			}
		});
}

NodeAdd::NodeAdd() : Node("add")
{
	color = add_input("channel");
	value = add_input("value");
	auto out = add_output("result");
	out->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			if (!value->input || !color->input)
			{
				out_value = -1;
				return;
			}

			float in_color = -1;
			float in_value = -1;
			color->input->on_get_value.execute(in_color, pos_x, pos_y, res_x, res_y);
			value->input->on_get_value.execute(in_value, pos_x, pos_y, res_x, res_y);
			out_value = in_color + in_value;
		});
	out->on_get_res.add_lambda(
		[&](float& res_x, float& res_y, float pos_x, float pos_y, float out_res_x, float out_res_y)
		{
			if (!color->input)
			{
				res_x = -1;
				res_y = -1;
				return;
			}

			color->input->on_get_res.execute(res_x, res_y, pos_x, pos_y, out_res_x, out_res_y);
		});
}

NodeMult::NodeMult() : Node("mult")
{
	color = add_input("channel");
	value = add_input("value");
	auto out = add_output("result");
	out->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			if (!value->input || !color->input)
			{
				out_value = -1;
				return;
			}

			float in_color = -1;
			float in_value = -1;
			color->input->on_get_value.execute(in_color, pos_x, pos_y, res_x, res_y);
			value->input->on_get_value.execute(in_value, pos_x, pos_y, res_x, res_y);
			out_value = in_color * in_value;
		});
	out->on_get_res.add_lambda(
		[&](float& res_x, float& res_y, float pos_x, float pos_y, float out_res_x, float out_res_y)
		{
			if (!color->input)
			{
				res_x = -1;
				res_y = -1;
				return;
			}

			color->input->on_get_res.execute(res_x, res_y, pos_x, pos_y, out_res_x, out_res_y);
		});
}
