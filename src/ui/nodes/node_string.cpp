#include "node_string.h"

#include <codecvt>
#include <iostream>

#include "file_actions.h"

#include <nfd.hpp>

/**
 * \brief Text
 */
TextInput::TextInput()
{
	const auto out = add_output("result");
	out->on_get_type.add_lambda([]
	{
		return EType::String;
	});
	out->on_get_code.add_lambda([&](CodeContext& context)
	{
		return value;
	});
}

void TextInput::display()
{
	char buf[2048];

	strcpy_s(buf, std::min(value.length() + 1, static_cast<size_t>(256)), value.c_str());
	if (ImGui::InputTextMultiline("value", buf, 2048, ImGui::GetContentRegionAvail(),
	                              ImGuiInputTextFlags_CtrlEnterForNewLine))
	{
		value = buf;
		mark_dirty();
	}
}

nlohmann::json TextInput::serialize(Graph& graph)
{
	auto js = Node::serialize(graph);
	js["value"] = value;
	return js;
}

void TextInput::deserialize(const nlohmann::json& json)
{
	Node::deserialize(json);
	if (json.contains("value"))
		value = json["value"];
}

/// <summary>
/// Directory
/// </summary>

DirectoryInput::DirectoryInput()
{
	const auto out = add_output("dir");
	out->on_get_type.add_lambda([]
	{
		return EType::String;
	});
	out->on_get_code.add_lambda([&](CodeContext& context)
	{
		return value;
	});
}


std::string wstring_to_string(std::wstring w_string)
{
	std::string ret;
	std::ranges::transform(w_string, std::back_inserter(ret),
	                       [](wchar_t c) { return static_cast<char>(c); });
	return ret;
}


void DirectoryInput::display()
{
	if (ImGui::Button((value + "##btn").c_str(), ImGui::GetContentRegionAvail()))
	{
		nfdnchar_t* outPath;
		if (NFD::PickFolder(outPath, nullptr) == NFD_OKAY)
		{
			value = wstring_to_string(outPath) + "/";
			NFD::FreePath(outPath);
			mark_dirty();
		}
	}
}

nlohmann::json DirectoryInput::serialize(Graph& graph)
{
	auto js = Node::serialize(graph);
	js["value"] = value;
	return js;
}

void DirectoryInput::deserialize(const nlohmann::json& json)
{
	Node::deserialize(json);
	if (json.contains("value"))
		value = json["value"];
}

AppendText::AppendText()
{
	a = add_input("A");
	b = add_input("B");
	out = add_output("Result");
	out->on_get_type.add_lambda([] { return EType::String; });
	out->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!*a && !*b)
		{
			return "";
		}

		if (!*b && *a)
		{
			return a->target()->get_code(get_graph().code_ctx());
		}
		if (!*a && *b)
		{
			return b->target()->get_code(get_graph().code_ctx());
		}

		const auto val = a->target()->get_code(get_graph().code_ctx()) + b->target()->get_code(get_graph().code_ctx());

		return val;
	});

	on_update.add_lambda([&]
	{
		value = out->on_get_code.execute(get_graph().code_ctx());
	});
}

void AppendText::display()
{
	ImGui::TextWrapped("%s", value.c_str());
}
