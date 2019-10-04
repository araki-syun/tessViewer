#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <boost\noncopyable.hpp>

#include <GL\glew.h>

//#include <opensubdiv\far\patchDescriptor.h>

#define GLAPP_GL_COMPILE_ERROR_EXCEPTION

namespace glapp{
	class glslshader : boost::noncopyable
	{
		friend class glslprogram;
	public:
		glslshader(void);
		glslshader(const std::string& filename);
		virtual ~glslshader(void);

		const GLuint GetShader() const;
		const std::string& GetSource() const;
		const GLuint GetType() const;
		const bool Empty() const;

		//static std::unordered_map<std::string, glslshader> shader_list;

	protected:
		//void Compile();
		//void includeReplace();

		GLuint _id;
		std::string _src;
		GLuint _type;

#ifndef GLAPP_GL_COMPILE_ERROR_EXCEPTION
		const std::string& GetError() const;
		std::string err;
#endif
		
	};
}

