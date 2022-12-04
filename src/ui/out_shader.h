#pragma once
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

class ShaderUniform;

class OutShader final
{
public:
	OutShader();
	~OutShader();
	void set_code(const std::string& code);
	void draw(ImVec4 clip_rect);
private:
	size_t last_hash = std::hash<std::string>{}(std::string(""));
	int glsl_program = 0;
	int glsl_vertex = 0;
	bool compiled = false;
	std::vector<std::shared_ptr<ShaderUniform>> uniforms;
};
