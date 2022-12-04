#include "node_string.h"

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
	char buf[256];
	strcpy_s(buf, std::min(value.length() + 1, static_cast<size_t>(256)), value.c_str());
	if (ImGui::InputText("value", buf, 256))
		value = buf;
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
