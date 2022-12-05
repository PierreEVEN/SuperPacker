#include "gfx.h"
#include "ui/graph.h"
#include "ui/nodes/node_constant.h"
#include "ui/nodes/node_maths.h"
#include "ui/nodes/node_string.h"
#include "ui/nodes/node_texture.h"
#include "ui/nodes/nodes_misc.h"

int main(int argc, char** argv)
{
	Gfx gfx("Super Packer v3.0", 800, 600);

	Graph::register_node<ImageWriteNode>();
	Graph::register_node<NodeAdd>();
	Graph::register_node<NodeMult>();
	Graph::register_node<NodeFloat>();
	Graph::register_node<NodeTexture>();
	Graph::register_node<NodeString>();
	Graph::register_node<AppendString>();
	Graph::register_node<MakeFloat4>();
	Graph::register_node<MakeFloat3>();
	Graph::register_node<MakeFloat2>();
	Graph::register_node<BreakColor>();

	Graph graph("test");

	gfx.on_draw.add_lambda([&]
	{
		FileResource::refresh_file_resources();
		graph.draw();
	});

	while (gfx.draw());
	graph.save_to_file();
}


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	return main(0, nullptr);
}
