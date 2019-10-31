#pragma once

#include <array>

#include <GL\glew.h>

#include <boost\noncopyable.hpp>

#include <opensubdiv\far\patchDescriptor.h>

#include "glslshader_manager.h"
#include "glShaderProgram.h"
#include "glapp_define.h"

namespace glapp {
class glslOSDprogram : glShaderProgram {
public:
	glslOSDprogram();
	~glslOSDprogram();

	//const GLuint GetProgram() const;

protected:
	//void create();
	GLuint             shaderCompile(const glslshader* shader) override;
	const std::string& includeReplace(const glslshader* shader);

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
