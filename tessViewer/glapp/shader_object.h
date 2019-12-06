#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <boost\noncopyable.hpp>

#include <GL\glew.h>

//#include <opensubdiv\far\patchDescriptor.h>

#define GLAPP_GL_COMPILE_ERROR_EXCEPTION

namespace glapp {
class Shader : boost::noncopyable {
	friend class glShaderProgram;

public:
	Shader();
	Shader(const std::string& filename);
	virtual ~Shader();

	GLuint      GetShader() const;
	std::string GetSource() const;
	GLuint      GetType() const;
	bool        Empty() const;

	//static std::unordered_map<std::string, glslshader> shader_list;

protected:
	//void Compile();
	//void includeReplace();

	GLuint      _id = 0;
	std::string _src;
	GLuint      _type = 0;

#ifndef GLAPP_GL_COMPILE_ERROR_EXCEPTION
	const std::string& GetError() const;
	std::string        err;
#endif
};
} // namespace glapp
