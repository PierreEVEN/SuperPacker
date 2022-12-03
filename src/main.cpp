#include <filesystem>
#include <imgui.h>

#include "gfx.h"
#include "ui/node.h"

int main(int argc, char** argv)
{
	Gfx gfx("Super Packer v3.0", 800, 600);

	Graph::register_node("ImageWrite", []() { return std::make_shared<ImageWriteNode>(); });
	Graph::register_node("Add", []() { return std::make_shared<NodeAdd>(); });
	Graph::register_node("Mult", []() { return std::make_shared<NodeMult>(); });
	Graph::register_node("Resize", []() { return std::make_shared<TextureResizeNode>(); });
	Graph::register_node("Constant", []() { return std::make_shared<NodeConstant>(); });
	Graph::register_node("Texture", []() { return std::make_shared<NodeTexture>(); });
	
	Graph graph("test");

	gfx.on_draw.add_lambda([&]
	{
		ImGui::ShowDemoWindow();
		graph.draw();
	});

	while (gfx.draw());
	graph.save_to_file();
}
