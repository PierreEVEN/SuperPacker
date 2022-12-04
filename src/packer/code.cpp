#include "code.h"

#include <format>

#include "gfx.h"

ShaderUniform::~ShaderUniform()
{
}

void ShaderUniform::unregister()
{
	if (!owner)
		return;
	for (size_t i = 0; i < owner->uniforms.size(); ++i)
		if (owner->uniforms[i].get() == this)
		{
			owner->uniforms.erase(owner->uniforms.begin() + i);
			break;
		}
	owner = nullptr;
}

std::shared_ptr<ShaderUniform> CodeContext::add_uniform(const EType& uniform_type)
{
	const auto uniform = std::shared_ptr<ShaderUniform>(
		new ShaderUniform(this, generate_name(), uniform_id++, uniform_type));
	uniforms.emplace_back(uniform);
	return uniform;
}

std::string CodeContext::generate_name()
{
	return std::string("_property_") + std::to_string(property_id++);
}

std::string CodeContext::glsl_type(EType type) const
{
	switch (type)
	{
	case EType::Undefined: return "Undefined";
	case EType::Float: return "float";
	case EType::Float2: return "vec2";
	case EType::Float3: return "vec3";
	case EType::Float4: return "vec4";
	case EType::Int: return "int";
	case EType::String: return "String";
	case EType::Sampler2D: return "sampler2D";
	default: return "UnhandledType";
	}
}

std::string CodeContext::generate_full_glsl(const std::string& code) const
{
	std::string uniforms_str;

	for (const auto& uniform : uniforms)
		uniforms_str += std::format("layout (location = {}) uniform {} {};\n", uniform->location,
		                            glsl_type(uniform->type),
		                            uniform->name);

	return std::format(
		"#version 430\n"
		"out vec4 output_color;\n"
		"layout (location = 0) in vec2 text_coords;\n"
		"{}\n"
		"void main() {{\n"
		"\toutput_color = vec4(0,0,0,1);\n"
		"\t{}\n"
		"}}\n",
		uniforms_str,
		code);
}

std::string CodeContext::glsl_output_var(EType type) const
{
	switch (type)
	{
	case EType::Undefined: return "undefined_type";
	case EType::Float: return "output_color.r";
	case EType::Float2: return "output_color.rg";
	case EType::Float3: return "output_color.rgb";
	case EType::Float4: return "output_color.rgba";
	default:
		return "incompatible_type";
	}
}

void CodeContext::update_uniforms() const
{
	for (const auto& uniform : uniforms) {
		GL_CHECK_ERROR();
		uniform->on_update_value(uniform->location);
		GL_CHECK_ERROR();
	}
}

CodeContext::~CodeContext()
{
}
