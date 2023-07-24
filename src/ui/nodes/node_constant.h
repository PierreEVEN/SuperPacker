#pragma once
#include "ui/node.h"

class Graph;

class NodeFloat : public Node
{
public:
	NodeFloat();
	virtual ~NodeFloat();
	void display(ESpTool tool) override;
	virtual void register_uniform(CodeContext& ctx) override;

	virtual nlohmann::json serialize(Graph& graph) override;
	void deserialize(const nlohmann::json& json) override;

	void display_summary() override;

	bool is_parameter() const override { return true; }
private:
	std::shared_ptr<ShaderUniform> uniform_var;
	float value = 0;
};
