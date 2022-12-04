#pragma once
#include <memory>
#include <string>
#include <vector>

#include "types.h"


class ShaderUniform
{
	friend class CodeContext;
public:
	ShaderUniform(std::string in_name, size_t in_location, const EType& in_type) : location(in_location),
		name(std::move(in_name)), type(in_type)
	{
	}

	virtual void bind() {};

private:
	const size_t location = 0;
	const std::string name;
	const EType type;
};

class CodeContext
{
public:
	std::shared_ptr<ShaderUniform> add_uniform(const std::string& uniform_name, const EType& uniform_type);
	
	[[nodiscard]] std::string generate_name();
	[[nodiscard]] std::string glsl_type(EType type) const;
	[[nodiscard]] std::string generate_full_glsl(const std::string& code) const;
	[[nodiscard]] std::string glsl_output_var(EType type) const;

private:
	std::vector<std::shared_ptr<ShaderUniform>> uniforms;
	size_t property_id = 0;
	size_t uniform_id = 1;
};
