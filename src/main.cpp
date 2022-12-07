#include "gfx.h"
#include "logger.h"
#include "ui/graph.h"
#include "ui/nodes/node_constant.h"
#include "ui/nodes/node_maths.h"
#include "ui/nodes/node_string.h"
#include "ui/nodes/node_texture.h"
#include "ui/nodes/nodes_misc.h"

#include "imgui_operators.h"
#include "ui/graph_browser.h"

REGISTER_NODE(ImageWriteNode, NodeInfo("", {"Image Write"}));
REGISTER_NODE(NodeAdd, NodeInfo("", {"Add", "+"}));
REGISTER_NODE(NodeMult, NodeInfo("", {"Mult", "*"}));
REGISTER_NODE(NodeFloat, NodeInfo("", {"Constant", "float"}));
REGISTER_NODE(NodeTexture, NodeInfo("", {"Texture"}));
REGISTER_NODE(TextInput, NodeInfo("", {"Text", "String"}));
REGISTER_NODE(AppendText, NodeInfo("", {"Append Text", "Concatenate"}));
REGISTER_NODE(MakeFloat4, NodeInfo("", {"Make Float4", "Append Float4", "Merge To Float 4"}));
REGISTER_NODE(MakeFloat3, NodeInfo("", {"Make Float3", "Append Float3", "Merge To Float 3"}));
REGISTER_NODE(MakeFloat2, NodeInfo("", {"Make Float2", "Append Float2", "Merge To Float 2"}));
REGISTER_NODE(BreakColor, NodeInfo("", {"Break Color", "Break Channels", "Split Channels"}));
REGISTER_NODE(DirectoryInput, NodeInfo("", {"Directory", "Select Directory"}));

int main(int argc, char** argv)
{
	Gfx gfx("Super Packer v3.0", 800, 600);

	GraphBrowser browser;
	browser.new_graph("test");
	browser.new_graph("Demo2");

	gfx.on_draw.add_lambda([&]
	{
		FileResource::refresh_file_resources();
		browser.display();
	});

	while (gfx.draw());
	browser.save_all();
}


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	return main(0, nullptr);
}
