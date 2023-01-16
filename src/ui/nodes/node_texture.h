#pragma once
#include "file_resource.h"
#include "ui/node.h"

class NodeTexture : public Node
{
public:
	NodeTexture();
	virtual ~NodeTexture();

	void display() override;
	void register_uniform(CodeContext& ctx) override;

	nlohmann::json serialize(Graph& graph) override;
	void deserialize(const nlohmann::json& json) override;

	void display_summary() override;
	ESummaryMode summary_mode() const override { return ESummaryMode::Input; }
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
	void display() override;
	ESummaryMode summary_mode() const override { return ESummaryMode::Output; }
	void display_summary() override;
private:
	std::filesystem::path default_path;
	std::shared_ptr<NodeInput> path_input;
	std::shared_ptr<NodeInput> rgba_input;
};
