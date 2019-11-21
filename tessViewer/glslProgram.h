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

class GlslProgram {
public:
	struct GlslInfo {
		GlslInfo();
		GlslInfo(std::string vert,
				 std::string frag,
				 std::string geom,
				 std::string tcs,
				 std::string tes);

		std::string Str() const;

		std::string vert, frag, geom, tcs, tes;

		static void CreateUniformBuffer(const std::string& name, int size);
		static const tv::GlShaderUniformBuffer*
		GetUniformBuffer(const std::string& name);

	private:
		static std::map<std::string, std::unique_ptr<tv::GlShaderUniformBuffer>>
			buffers;
	};
	GlslProgram();
	GlslProgram(const GlslInfo& glsl);
	GlslProgram(const GlslInfo& glsl, const OsdInfo& osd);
	~GlslProgram();

	GLuint GetProgram() const;
	void   SetProgram(const GlslInfo& glsl);
	void   SetProgram(const GlslInfo& glsl, const OsdInfo& osd);
	void   SetProgram(const std::string& vert, const std::string& frag);
	const tv::GlShaderAttribute* GetAttrib(const std::string& name) const;
	const tv::GlShaderUniform*   GetUniform(const std::string& name) const;
	const tv::GlShaderUniformBlock*
	GetUniformBlock(const std::string& name) const;
	//void BindUniformBlock();
private:
	void _set_location();

	GLuint _program{};

	std::map<std::string, std::unique_ptr<tv::GlShaderAttribute>> _attrib_map;
	std::map<std::string, std::unique_ptr<tv::GlShaderUniform>>   _uniform_map;
	std::map<std::string, std::unique_ptr<tv::GlShaderUniformBlock>>
		  _uniform_block_map;
	GLint _fragment = -1;
};
