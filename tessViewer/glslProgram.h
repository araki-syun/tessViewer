#pragma once

#include <string>
#include <map>
#include <memory>
#include <GL\glew.h>

#include "opensubdiv\osd\opengl.h"
#include "opensubdiv\osd\glslPatchShaderSource.h"
#include "opensubdiv\far\patchDescriptor.h"

#include "glShaderVariable.h"
#include "location_define.h"
#include "osd_info.h"

#define GLSL_VERSION "#version 450\n"
#define DEFAULT_VERTEX_SHADER SHADER "default.vert"
#define DEFAULT_FRAGMENT_SHADER SHADER "default.frag"
#define DEFAULT_GEOMETRY_SHADER SHADER "default.geom"
#define DEFAULT_TESS_CTRL_SHADER SHADER "default.tcs"
#define DEFAULT_TESS_EVAL_SHADER SHADER "default.tes"

class glslProgram {
public:
	struct glsl_info {
		glsl_info();
		glsl_info(const glsl_info& info);
		glsl_info(std::string vert,
				  std::string frag,
				  std::string geom,
				  std::string tcs,
				  std::string tes);

		std::string str() const;

		std::string vert, frag, geom, tcs, tes;

		static void CreateUniformBuffer(const std::string& name, int size);
		static const tv::glShaderUniformBuffer*
		GetUniformBuffer(const std::string& name);

	private:
		static std::map<std::string, std::unique_ptr<tv::glShaderUniformBuffer>>
			buffers;
	};
	glslProgram();
	glslProgram(const glsl_info& glsl);
	glslProgram(const glsl_info& glsl, const osd_info& osd);
	~glslProgram();

	GLuint GetProgram() const;
	void   SetProgram(const glsl_info& glsl);
	void   SetProgram(const glsl_info& glsl, const osd_info& osd);
	void   SetProgram(const std::string& vert, const std::string& frag);
	const tv::glShaderAttribute* GetAttrib(const std::string& name) const;
	const tv::glShaderUniform*   GetUniform(const std::string& name) const;
	const tv::glShaderUniformBlock*
	GetUniformBlock(const std::string& name) const;
	//void BindUniformBlock();
private:
	void SetLocation();

	GLuint _program{};
	
	std::map<std::string, std::unique_ptr<tv::glShaderAttribute>> attrib_map;
	std::map<std::string, std::unique_ptr<tv::glShaderUniform>>   uniform_map;
	std::map<std::string, std::unique_ptr<tv::glShaderUniformBlock>>
		  uniformBlock_map;
	GLint fragment = -1;
};
