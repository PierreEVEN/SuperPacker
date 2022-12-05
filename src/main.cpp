
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

	Graph::register_node("ImageWrite", []() { return std::make_shared<ImageWriteNode>(); });
	Graph::register_node("Add", []() { return std::make_shared<NodeAdd>(); });
	Graph::register_node("Mult", []() { return std::make_shared<NodeMult>(); });
	Graph::register_node("Constant", []() { return std::make_shared<NodeFloat>(); });
	Graph::register_node("Texture", []() { return std::make_shared<NodeTexture>(); });
	Graph::register_node("String", []() { return std::make_shared<NodeString>(); });
	Graph::register_node("Append String", []() { return std::make_shared<AppendString>(); });
	Graph::register_node("Make Float4", []() { return std::make_shared<MakeFloat4>(); });
	Graph::register_node("Make Float3", []() { return std::make_shared<MakeFloat3>(); });
	Graph::register_node("Make Float2", []() { return std::make_shared<MakeFloat2>(); });
	Graph::register_node("Break Color", []() { return std::make_shared<BreakColor>(); });
	
	Graph graph("test");

	gfx.on_draw.add_lambda([&]
	{
		graph.draw();
	});

	while (gfx.draw());
	graph.save_to_file();
}


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	return main(0, nullptr);
}