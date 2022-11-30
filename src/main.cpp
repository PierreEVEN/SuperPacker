#include <filesystem>
#include <imgui.h>

#include "gfx.h"
#include "texture.h"
#include "packer/packer.h"
#include "ui/node.h"

int main(int argc, char** argv)
{
	Gfx gfx("Super Packer v3.0", 800, 600);

	Packer packer;

	Graph graph;

	graph.create_node<NodeTexture>(Texture::create(std::filesystem::path("resources/icon.png")));
	graph.create_node<ImageWriteNode>();
	graph.create_node<NodeAdd>();
	graph.create_node<NodeMult>();
	graph.create_node<TextureResizeNode>();
	graph.create_node<NodeConstant>(0.f);
	graph.create_node<NodeConstant>(1.f);
	graph.create_node<NodeConstant>(2.f);
	graph.create_node<NodeConstant>(3.f);
	graph.create_node<NodeConstant>(4.f);
	graph.create_node<NodeConstant>(5.f);

	gfx.on_draw.add_lambda([&]
	{
		graph.draw();
	});

	while (gfx.draw());
}
