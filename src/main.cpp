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

int main(int argc, char** argv)
{
	Gfx gfx("Super Packer v3.0", 800, 600);

	Graph::register_node<ImageWriteNode>();
	Graph::register_node<NodeAdd>();
	Graph::register_node<NodeMult>();
	Graph::register_node<NodeFloat>();
	Graph::register_node<NodeTexture>();
	Graph::register_node<TextInput>();
	Graph::register_node<AppendText>();
	Graph::register_node<MakeFloat4>();
	Graph::register_node<MakeFloat3>();
	Graph::register_node<MakeFloat2>();
	Graph::register_node<BreakColor>();
	Graph::register_node<DirectoryInput>();

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
