#include "node.h"

#include <iostream>

#include "texture.h"

static ImVec2 operator+(const ImVec2& left, const ImVec2& right)
{
	return {left.x + right.x, left.y + right.y};
}

static ImVec2 operator-(const ImVec2& left, const ImVec2& right)
{
	return {left.x - right.x, left.y - right.y};
}

static void draw_connection(ImVec2 from, ImVec2 to)
{
	const float delta = std::abs(from.x - ImGui::GetMousePos().x) * 0.75f;
	ImGui::GetForegroundDrawList()->AddBezierCurve(from,
	                                               from + ImVec2(delta, 0),
	                                               to + ImVec2(-delta, 0),
	                                               to,
	                                               ImGui::ColorConvertFloat4ToU32({1, 1, 0, 1}), 2);
}

void Graph::draw()
{
	for (const auto& node : nodes)
		node->display_internal(*this);

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		connection_output_to_input = nullptr;
		if (connection_input_to_output && connection_input_to_output->input)
			connection_input_to_output->input = nullptr;
		connection_input_to_output = nullptr;
	}

	if (connection_output_to_input)
	{
		draw_connection(connection_output_to_input->position, ImGui::GetMousePos());
	}

	if (connection_input_to_output)
	{
		draw_connection(ImGui::GetMousePos(), connection_input_to_output->position);
	}
}

void Graph::begin_out_in(std::shared_ptr<NodeOutput> start)
{
	if (connection_output_to_input == nullptr)
		connection_output_to_input = start;
}

void Graph::end_out_in(std::shared_ptr<NodeInput> end)
{
	end->input = connection_output_to_input;
	connection_output_to_input = nullptr;
}

void Graph::begin_in_out(std::shared_ptr<NodeInput> start)
{
	connection_input_to_output = start;
}

void Graph::end_in_out(std::shared_ptr<NodeOutput> end)
{
	if (connection_input_to_output)
		connection_input_to_output->input = end;
	connection_input_to_output = nullptr;
}

static size_t node_uuids = 0;

Node::Node(const std::string in_name) : name(in_name)
{
	uuid = node_uuids++;
}

void Node::display_internal(Graph& graph)
{
	if (ImGui::Begin((std::string("node_") + std::to_string(uuid)).c_str(), nullptr, ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::Text(name.c_str());
		display();

		const ImVec2 dims = ImGui::GetWindowSize();

		{
			const float step = dims.y / outputs.size();
			float pos = dims.y / 2 - (step / 2 * (outputs.size() - 1));
			for (auto& output : outputs)
			{
				output->position = ImGui::GetWindowPos() + ImVec2{dims.x, pos};
				const bool hover = ImGui::IsMouseHoveringRect(output->position - ImVec2{20, step / 2},
				                                              output->position + ImVec2{20, step / 2}, false);
				ImGui::GetWindowDrawList()->AddCircleFilled(output->position, hover ? 10.f : 5.f,
				                                            hover
					                                            ? ImGui::ColorConvertFloat4ToU32({1, 1, 0, 1})
					                                            : ImGui::ColorConvertFloat4ToU32({0.5, 0.5, 0, 1}));
				ImGui::GetWindowDrawList()->AddText(
					output->position - ImVec2{ImGui::CalcTextSize(output->name.c_str()).x + 10, 7},
					ImGui::ColorConvertFloat4ToU32({1, 1, 1, 1}), output->name.c_str());

				pos += step;

				if (hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					graph.begin_out_in(output);
				}

				if (hover && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					graph.end_in_out(output);
				}
			}
		}
		{
			const float step = dims.y / inputs.size();
			float pos = dims.y / 2 - (step / 2 * (inputs.size() - 1));
			for (auto& input : inputs)
			{
				input->position = ImGui::GetWindowPos() + ImVec2{0, pos};
				const bool hover = ImGui::IsMouseHoveringRect(input->position - ImVec2{20, step / 2},
				                                              input->position + ImVec2{20, step / 2}, false);

				ImGui::GetWindowDrawList()->AddCircleFilled(input->position, hover ? 10.f : 5.f,
				                                            hover
					                                            ? ImGui::ColorConvertFloat4ToU32({1, 1, 0, 1})
					                                            : ImGui::ColorConvertFloat4ToU32({0.5, 0.5, 0, 1}));

				ImGui::GetWindowDrawList()->AddText(
					input->position + ImVec2{10, -7},
					ImGui::ColorConvertFloat4ToU32({1, 1, 1, 1}), input->name.c_str());

				pos += step;

				if (hover && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					graph.end_out_in(input);
				}

				if (hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					graph.begin_in_out(input);
				}

				if (input->input)
				{
					const float delta = std::abs(input->input->position.x - input->position.x) * 0.75f;
					ImGui::GetForegroundDrawList()->AddBezierCurve(
						input->input->position,
						input->input->position + ImVec2(delta, 0),
						input->position + ImVec2(-delta, 0),
						input->position,
						ImGui::ColorConvertFloat4ToU32({1, 1, 0, 1}), 2);
				}
			}
		}
	}
	ImGui::End();
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
	auto output = std::make_shared<NodeOutput>();
	output->name = name;
	outputs.emplace_back(output);
	return output;
}

void NodeTexture::display()
{
	ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<size_t>(texture->get_id())), ImGui::GetContentRegionAvail());
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

NodeTexture::NodeTexture(std::shared_ptr<Texture> in_texture)
	: Node("Texture"), texture(in_texture)
{
	r = add_output("R");
	r->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_value = texture->get_color(0, pos_x, pos_y);
		});
	r->on_get_res.add_lambda(
		[&](float& out_x, float& out_y, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_x = static_cast<float>(texture->res_x());
			out_y = static_cast<float>(texture->res_y());
		});

	g = add_output("G");
	g->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_value = texture->get_color(1, pos_x, pos_y);
		});
	g->on_get_res.add_lambda(
		[&](float& out_x, float& out_y, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_x = static_cast<float>(texture->res_x());
			out_y = static_cast<float>(texture->res_y());
		});

	b = add_output("B");
	b->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_value = texture->get_color(2, pos_x, pos_y);
		});
	b->on_get_res.add_lambda(
		[&](float& out_x, float& out_y, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_x = static_cast<float>(texture->res_x());
			out_y = static_cast<float>(texture->res_y());
		});

	a = add_output("A");
	a->on_get_value.add_lambda(
		[&](float& out_value, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_value = texture->get_color(3, pos_x, pos_y);
		});
	a->on_get_res.add_lambda(
		[&](float& out_x, float& out_y, float pos_x, float pos_y, float res_x, float res_y)
		{
			out_x = static_cast<float>(texture->res_x());
			out_y = static_cast<float>(texture->res_y());
		});
}

NodeConstant::NodeConstant(float in_value)
	: Node("Constant"), value(in_value)
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
