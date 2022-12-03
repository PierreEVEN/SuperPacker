
#include "gfx.h"
#include "ui/graph.h"
#include "ui/nodes/node_constant.h"
#include "ui/nodes/node_maths.h"
#include "ui/nodes/node_string.h"
#include "ui/nodes/node_texture.h"

int main(int argc, char** argv)
{
	Gfx gfx("Super Packer v3.0", 800, 600);

	Graph::register_node("ImageWrite", []() { return std::make_shared<ImageWriteNode>(); });
	Graph::register_node("Add", []() { return std::make_shared<NodeAdd>(); });
	Graph::register_node("Mult", []() { return std::make_shared<NodeMult>(); });
	Graph::register_node("Resize", []() { return std::make_shared<TextureResizeNode>(); });
	Graph::register_node("Constant", []() { return std::make_shared<NodeFloat>(); });
	Graph::register_node("Texture", []() { return std::make_shared<NodeTexture>(); });
	Graph::register_node("String", []() { return std::make_shared<NodeString>(); });
	
	Graph graph("test");

	gfx.on_draw.add_lambda([&]
	{
		graph.draw();
	});

	while (gfx.draw());
	graph.save_to_file();
}
