#pragma once

#include <unordered_map>
#include <array>
#include <initializer_list>

#include <GL\glew.h>

#include "shader_manager.h"

namespace glapp {
class ShaderProgram {
public:
	ShaderProgram();
	ShaderProgram(std::initializer_list<std::string> list);
	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram(ShaderProgram&& prog) noexcept;
	~ShaderProgram();
	ShaderProgram& operator=(const ShaderProgram&) = delete;
	ShaderProgram& operator=(ShaderProgram&& prog) noexcept;

	GLuint GetProgram() const;
	void   SetShaderName(std::initializer_list<std::string> list);
	GLint  GetIndexAttrib(const std::string& name) const;
	GLint  GetIndexUniform(const std::string& name) const;

protected:
	void           _Create();
	virtual GLuint _ShaderCompile(const Shader* shader);
	void           _SetIndexUniform();
	void           _SetIndexAttrib();

protected:
	GLuint _program = 0;

	std::array<std::string, 5>             _shader_list;
	std::unordered_map<std::string, GLint> _attrib_list, _uniform_list;

	enum ShaderType : int {
		Vertex = 0,
		Fragment,
		Geometry,
		Tess_Control,
		Tess_Eval
	};

#ifndef GLAPP_GL_COMPILE_ERROR_EXCEPTION
	std::string GetError() const;
	std::string err;
#endif
};
} // namespace glapp
