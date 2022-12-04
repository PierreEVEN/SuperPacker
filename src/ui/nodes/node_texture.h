#pragma once
#include "ui/node.h"

class ImageWriteNode : public Node
{
public:
	ImageWriteNode() : Node("Image Write")
	{
		r = add_input("R");
		g = add_input("G");
		b = add_input("B");
		a = add_input("A");
	}

	void display() override;
private:
	std::shared_ptr<NodeInput> r;
	std::shared_ptr<NodeInput> g;
	std::shared_ptr<NodeInput> b;
	std::shared_ptr<NodeInput> a;
};

class NodeTexture : public Node
{
public:
	NodeTexture();

	void display() override;
private:
	std::shared_ptr<NodeInput> path;
	void load_or_reload();
	std::shared_ptr<Texture> texture;
};

class TextureResizeNode : public Node
{
public:
	TextureResizeNode();

	void display() override
	{
	}

private:
	std::shared_ptr<NodeInput> color;
	std::shared_ptr<NodeInput> in_x;
	std::shared_ptr<NodeInput> in_y;
};
