#pragma once

#include <array>

#include <GL\glew.h>

#include <boost\noncopyable.hpp>

#include <opensubdiv\far\patchDescriptor.h>

#include "shader_manager.h"
#include "shader_program.h"
#include "glapp_define.h"

namespace glapp {
class OSDProgram : ShaderProgram {
public:
	OSDProgram();
	~OSDProgram();

	//const GLuint GetProgram() const;

protected:
	//void create();
	GLuint      _ShaderCompile(const Shader* shader) override;
	std::string _IncludeReplace(const Shader* shader);

protected:
	//GLuint _program;
	//std::array<std::string, 5> shader_list;
	OpenSubdiv::Far::PatchDescriptor::Type _type;

#ifndef GLAPP_GL_COMPILE_ERROR_EXCEPTION
	const std::string& GetError() const;
	std::string        err;
#endif
};
} // namespace glapp
