#include "out_shader.h"

#include <iostream>
#include <GL/gl3w.h>

#include "gfx.h"
#include "packer/code.h"

OutShader::OutShader()
{
	last_hash = std::hash<std::string>{}(std::string(""));

	glsl_program = glCreateProgram();
	glsl_vertex = glCreateShader(GL_VERTEX_SHADER);
	const char* src =
		"#version 430\n"
		"layout(location = 0) out vec2 text_coords;\n"
		"layout(location = 1) uniform vec4 clip_rect;\n"
		"void main() {\n"
		"   text_coords = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);\n"
		"	vec2 pos = text_coords * (clip_rect.zw - clip_rect.xy) + clip_rect.xy;"	
		"	gl_Position = vec4(pos * 2 - 1, 0, 1);\n"
		"}\n";
	glShaderSource(glsl_vertex, 1, &src, nullptr);
	glCompileShader(glsl_vertex);

	int infologLength = 0;
	glGetShaderiv(glsl_vertex, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 1)
	{
		char* infoLog = new char[infologLength];
		int charsWritten = 0;
		glGetShaderInfoLog(glsl_vertex, infologLength, &charsWritten, infoLog);
		std::cout << "failed to compile vertex shader :" << std::endl << infoLog << std::endl;
		delete[] infoLog;
	}
	GL_CHECK_ERROR();
}

OutShader::~OutShader()
{
	glDeleteProgram(glsl_program);
	glDeleteShader(glsl_vertex);
}

void OutShader::set_code(const std::string& code)
{
	const size_t new_hash = std::hash<std::string>{}(std::string(code));
	GL_CHECK_ERROR();
	if (new_hash != last_hash)
	{
		compiled = false;
		last_hash = new_hash;

		int glsl_fragment = glCreateShader(GL_FRAGMENT_SHADER);
		const char* src = code.c_str();
		glShaderSource(glsl_fragment, 1, &src, nullptr);
		glCompileShader(glsl_fragment);

		int infologLength = 0;
		glGetShaderiv(glsl_fragment, GL_INFO_LOG_LENGTH, &infologLength);
		if (infologLength > 1)
		{
			char* infoLog = new char[infologLength];
			int charsWritten = 0;
			glGetShaderInfoLog(glsl_fragment, infologLength, &charsWritten, infoLog);
			std::cout << "failed to compile fragment shader :" << std::endl << infoLog << std::endl <<
				"__________________________" << std::endl << code << std::endl << "__________________________" <<
				std::endl;
			delete[] infoLog;
			compiled = false;
			return;
		}
		GL_CHECK_ERROR();

		glAttachShader(glsl_program, glsl_vertex);
		glAttachShader(glsl_program, glsl_fragment);
		glLinkProgram(glsl_program);
		GL_CHECK_ERROR();

		compiled = true;
		glDetachShader(glsl_program, glsl_fragment);
		glDetachShader(glsl_program, glsl_vertex);
		glDeleteShader(glsl_fragment);
		GL_CHECK_ERROR();

		glGetShaderiv(glsl_program, GL_INFO_LOG_LENGTH, &infologLength);
		while (glGetError());
		GL_CHECK_ERROR();
		if (infologLength > 1)
		{
			char* infoLog = new char[infologLength];
			int charsWritten = 0;
			glGetShaderInfoLog(glsl_program, infologLength, &charsWritten, infoLog);
			GL_CHECK_ERROR();
			std::cout << "failed to link program:" << std::endl << infoLog << std::endl;
			delete[] infoLog;
			compiled = false;
		}
		GL_CHECK_ERROR();
	}


	GL_CHECK_ERROR();
}

bool OutShader::bind(ImVec4 clip_rect)
{
	if (!compiled)
		return false;
	GL_CHECK_ERROR();
	glUseProgram(glsl_program);
	GL_CHECK_ERROR();
	return true;
}

void OutShader::draw()
{
	if (!compiled)
		return;

	GL_CHECK_ERROR();
	glDrawArrays(GL_TRIANGLES, 0, 3);
	GL_CHECK_ERROR();
}
