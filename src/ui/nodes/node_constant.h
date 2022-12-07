#pragma once
#include "ui/node.h"

class Graph;

class NodeFloat : public Node
{
public:
	NodeFloat();
	virtual ~NodeFloat();
	void display() override;
	virtual void register_uniform(CodeContext& ctx) override;

	virtual nlohmann::json serialize(Graph& graph) override;
	void deserialize(const nlohmann::json& json) override;

	ESummaryMode summary_mode() const override { return ESummaryMode::Input; }

private:
	std::shared_ptr<ShaderUniform> uniform_var;
	float value;
};
