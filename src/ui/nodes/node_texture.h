#pragma once
#include "ui/node.h"

class NodeTexture : public Node
{
public:
	NodeTexture();
	virtual ~NodeTexture();

	void display() override;
	void register_uniform(CodeContext& ctx) override;
private:
	std::shared_ptr<NodeInput> path;
	void load_or_reload();
	std::shared_ptr<Texture> texture;
	std::shared_ptr<ShaderUniform> texture_uniform;
	std::shared_ptr<ShaderUniform> enabled_uniform;
};

class ImageWriteNode : public Node
{
public:
	ImageWriteNode();
	void display() override;
private:
	std::shared_ptr<NodeInput> path_input;
	std::shared_ptr<NodeInput> rgba_input;
};
