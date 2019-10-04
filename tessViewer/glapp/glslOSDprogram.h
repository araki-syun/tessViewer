#pragma once

#include <array>

#include <GL\glew.h>

#include <boost\noncopyable.hpp>

#include <opensubdiv\far\patchDescriptor.h>

#include "glslshader_manager.h"
#include "glslprogram.h"
#include "glapp_define.h"

namespace glapp{
	class glslOSDprogram : glslprogram
	{
	public:
		glslOSDprogram(void);
		~glslOSDprogram(void);

		//const GLuint GetProgram() const;


	protected:
		//void create();
		const GLuint shaderCompile(const glslshader* shader) override;
		const std::string& includeReplace(const glslshader* shader);

	protected:
		//GLuint _program;
		//std::array<std::string, 5> shader_list;
		OpenSubdiv::Far::PatchDescriptor::Type _type;

#ifndef GLAPP_GL_COMPILE_ERROR_EXCEPTION
		const std::string& GetError() const;
		std::string err;
#endif
	
	};
}

