#pragma once
#include "file_resource.h"
#include "ui/node.h"

class NodeTexture : public Node
{
public:
	NodeTexture();
	virtual ~NodeTexture();

	void display(ESpTool tool) override;
	void register_uniform(CodeContext& ctx) override;

	nlohmann::json serialize(Graph& graph) override;
	void deserialize(const nlohmann::json& json) override;

	void display_summary() override;
	bool is_parameter() const override { return true; }
private:
	std::shared_ptr<Texture> texture;
	std::shared_ptr<ShaderUniform> texture_uniform;
	std::shared_ptr<ShaderUniform> enabled_uniform;

	FileResource texture_data;
};

class ImageWriteNode : public Node
{
public:
	ImageWriteNode();
	void display(ESpTool tool) override;
	void display_summary() override;
	bool is_parameter() const override { return true; }
private:
	std::filesystem::path default_path;
	std::shared_ptr<InputPin> path_input;
	std::shared_ptr<InputPin> rgba_input;
};
