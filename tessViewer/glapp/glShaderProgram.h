#pragma once

#include <unordered_map>
#include <array>
#include <initializer_list>

#include <GL\glew.h>

#include <boost\noncopyable.hpp>

#include "glslshader_manager.h"

namespace glapp {
class glShaderProgram : boost::noncopyable {
public:
	glShaderProgram();
	glShaderProgram(std::initializer_list<std::string> list);
	~glShaderProgram();

	GLuint       GetProgram() const;
	void         SetShaderName(std::initializer_list<std::string> list);
	GLint        GetIndexAttrib(const std::string& name) const;
	GLint        GetIndexUniform(const std::string& name) const;

protected:
	void                 create();
	virtual GLuint       shaderCompile(const glslshader* shader);
	void                 setIndexUniform();
	void                 setIndexAttrib();

protected:
	GLuint _program = 0;

	std::array<std::string, 5>             shader_list;
	std::unordered_map<std::string, GLint> attribList, uniformList;

	enum shader_type : int {
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
