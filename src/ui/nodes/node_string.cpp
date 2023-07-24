#include "node_string.h"

#include <codecvt>

#include <nfd.hpp>

#include "ui/pin.h"

static std::string wstring_to_string(std::wstring w_string)
{
	std::string ret;
	std::ranges::transform(w_string, std::back_inserter(ret),
	                       [](wchar_t c) { return static_cast<char>(c); });
	return ret;
}


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

void TextInput::display(ESpTool tool)
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

void TextInput::display_summary()
{
	Node::display_summary();

	char buf[2048];

	strcpy_s(buf, std::min(value.length() + 1, static_cast<size_t>(256)), value.c_str());
	if (ImGui::InputTextMultiline(("value##" + uuid_as_string()).c_str(), buf, 2048, {ImGui::GetContentRegionAvail().x, 100},
	                              ImGuiInputTextFlags_CtrlEnterForNewLine))
	{
		value = buf;
		mark_dirty();
	}
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


void DirectoryInput::display(ESpTool tool)
{
	if (ImGui::Button((value + "##btn" + uuid_as_string()).c_str(), ImGui::GetContentRegionAvail()))
	{
		nfdnchar_t* outPath;
		if (NFD::PickFolder(outPath, nullptr) == NFD_OKAY)
		{
			value = std::filesystem::relative(wstring_to_string(outPath)).string() + "/";
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

void DirectoryInput::display_summary()
{
	Node::display_summary();
	if (ImGui::Button((value + "##btn_" + uuid_as_string()).c_str(), {ImGui::GetContentRegionAvail().x, 100}))
	{
		nfdnchar_t* outPath;
		if (NFD::PickFolder(outPath, nullptr) == NFD_OKAY)
		{
			value = std::filesystem::relative(wstring_to_string(outPath)).string() + "/";
			NFD::FreePath(outPath);
			mark_dirty();
		}
	}
}

FileInput::FileInput()
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

void FileInput::display(ESpTool tool)
{
	if (ImGui::Button((value + "##btn").c_str(), ImGui::GetContentRegionAvail()))
	{
		nfdnchar_t* outPath;
		if (NFD::OpenDialog(outPath) == NFD_OKAY)
		{
			value = std::filesystem::relative(wstring_to_string(outPath)).string();
			NFD::FreePath(outPath);
			mark_dirty();
		}
	}
}

nlohmann::json FileInput::serialize(Graph& graph)
{
	auto js = Node::serialize(graph);
	js["value"] = value;
	return js;
}

void FileInput::deserialize(const nlohmann::json& json)
{
	Node::deserialize(json);
	if (json.contains("value"))
		value = json["value"];
}

void FileInput::display_summary()
{
	Node::display_summary();
	
	if (ImGui::Button((value + "##btn" + uuid_as_string()).c_str(), {ImGui::GetContentRegionAvail().x, 100}))
	{
		nfdnchar_t* outPath;
		if (NFD::OpenDialog(outPath) == NFD_OKAY)
		{
			value = std::filesystem::relative(wstring_to_string(outPath)).string();
			NFD::FreePath(outPath);
			mark_dirty();
		}
	}
}

AppendText::AppendText()
{
	a = add_input("A");
	b = add_input("B");
	out = add_output("Result");
	out->on_get_type.add_lambda([] { return EType::String; });
	out->on_get_code.add_lambda([&](CodeContext& context)-> std::string
	{
		if (!a->is_connected() && !b->is_connected())
		{
			return "";
		}

		if (!b->is_connected() && a->is_connected())
		{
			return a->target()->get_code(get_graph().code_ctx());
		}
		if (!a->is_connected() && b->is_connected())
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

void AppendText::display(ESpTool tool)
{
	ImGui::TextWrapped("%s", value.c_str());
}

REGISTER_NODE(TextInput, NodeInfo("", { "Text", "String" }));
REGISTER_NODE(AppendText, NodeInfo("", { "Append Text", "Concatenate" }));
REGISTER_NODE(DirectoryInput, NodeInfo("", { "Directory", "Select Directory" }));
REGISTER_NODE(FileInput, NodeInfo("", { "File", "Select File" }));
