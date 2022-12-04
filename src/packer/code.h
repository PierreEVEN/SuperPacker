#pragma once
#include <memory>
#include <string>
#include <vector>

#include "types.h"
#include <event_manager.h>

DECLARE_DELEGATE_MULTICAST(EventUpdateUniform, int)

class ShaderUniform
{
	friend class CodeContext;
public:
	virtual void bind() {}

	EventUpdateUniform on_update_value;
	const std::string& get_name() const { return name; }
	~ShaderUniform();
	void unregister();

private:
	ShaderUniform(CodeContext* in_owner, std::string in_name, int in_location, const EType& in_type) : owner(in_owner),
		location(in_location), name(std::move(in_name)), type(in_type)
	{
	}
	CodeContext* owner;
	const int location = 0;
	const std::string name;
	const EType type;
};

class CodeContext
{
	friend class ShaderUniform;
public:
	std::shared_ptr<ShaderUniform> add_uniform(const EType& uniform_type);
	
	[[nodiscard]] std::string generate_name();
	[[nodiscard]] std::string glsl_type(EType type) const;
	[[nodiscard]] std::string generate_full_glsl(const std::string& code) const;
	[[nodiscard]] std::string glsl_output_var(EType type) const;

	void update_uniforms() const;
	virtual ~CodeContext();
private:
	std::vector<std::shared_ptr<ShaderUniform>> uniforms;
	size_t property_id = 0;
	int uniform_id = 1;
};
