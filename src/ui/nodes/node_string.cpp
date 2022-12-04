#include "node_string.h"

#include <iostream>

NodeString::NodeString() : Node("String")
{
	const auto out = add_output("text");
	out->on_get_type.add_lambda([]
	{
		return EType::String;
	});
	out->on_get_code.add_lambda([&](CodeContext& context)
	{
		return value;
	});
}

void NodeString::display()
{
	char buf[2048];
	strcpy_s(buf, std::min(value.length() + 1, static_cast<size_t>(256)), value.c_str());
	if (ImGui::InputText("value", buf, 2048)) {
		value = buf;
	}
}

nlohmann::json NodeString::serialize(Graph& graph)
{
	auto js = Node::serialize(graph);
	js["value"] = value;
	return js;
}

void NodeString::deserialize(const nlohmann::json& json)
{
	Node::deserialize(json);
	if (json.contains("value"))
		value = json["value"];
}

AppendString::AppendString() : Node("Append String")
{
	a = add_input("A");
	b = add_input("B");
	const auto out = add_output("Result");
	out->on_get_type.add_lambda([] { return EType::String; });
	out->on_get_code.add_lambda([&](CodeContext& context)-> std::string
		{
			if (!*b && *a)
				return a->target()->get_code(context);
			if (!*a && *b)
				return b->target()->get_code(context);

			if (!*a && !*b)
				return "";

			return a->target()->get_code(context) + b->target()->get_code(context);
		});
}
