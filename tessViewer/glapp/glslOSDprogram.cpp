#include "glslOSDprogram.h"

#include <regex>
#include <boost\format.hpp>

#include <opensubdiv\osd\glslPatchShaderSource.h>

#include "glapp_define.h"

namespace glapp {
glslOSDprogram::glslOSDprogram()
	: _type(OpenSubdiv::Far::PatchDescriptor::Type::REGULAR) {}

glslOSDprogram::~glslOSDprogram() { glDeleteProgram(_program); }

//const GLuint glslOSDprogram::GetProgram() const{
//	return _program;
//}

//	void glslOSDprogram::create(){
//		_program = glCreateProgram();
//
//		for(std::string& str : shader_list){
//			glAttachShader(_program, shaderCompile(glslshader_manager::GetPointer(str)));
//		}
//		glLinkProgram(_program);
//
//		GLint result;
//		glGetProgramiv(_program, GL_LINK_STATUS, &result);
//		if (result == GL_FALSE) {
//			// �T�C�Y���擾
//			GLint log_length;
//			glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &log_length);
//
//			// ��������擾
//			GLsizei max_length;
//			GLsizei length;
//			std::vector<GLchar> log(max_length);
//			glGetProgramInfoLog(_program, max_length, &length, log.data());
//
//#ifdef GLAPP_GL_COMPILE_ERROR_EXCEPTION
//			std::string err;
//			err.append(log.data(), log.size());
//
//			throw std::runtime_error((boost::format("Shader Program Link ERROR\n%1%") % err).str());
//#else
//			err.append(log.data(), log.size());
//#endif
//		}
//	}

GLuint glslOSDprogram::shaderCompile(const glslshader* shader) {
	GLuint      id(0);
	const char* str = includeReplace(shader).c_str();
	id              = glCreateShader(shader->GetType());
	glShaderSource(id, 1, &str, nullptr);
	glCompileShader(id);

	// ���ۂ��m�F
	GLint result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);

	// ���O���擾
	if (result == GL_FALSE) {
		// �T�C�Y���擾
		GLint log_length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);

		// ��������擾
		GLsizei             max_length(log_length);
		GLsizei             length;
		std::vector<GLchar> log(max_length);
		glGetShaderInfoLog(id, max_length, &length, log.data());

#ifdef GLAPP_GL_COMPILE_ERROR_EXCEPTION
		std::string err;
		err.append(log.data(), log.size());

		throw std::runtime_error(
			(boost::format("Shader Compile ERROR\n%1%") % err).str());
#else
		err.append(log.data(), log.size());
#endif
		return id;
	}
	return 0;
}

const std::string& glslOSDprogram::includeReplace(const glslshader* shader) {
	using namespace OpenSubdiv::Osd;

	std::string src;
	std::regex  common_include("#include <" GLAPP_OSD_COMMON_SOURCE_STRING ">");
	std::regex  vertex_include("#include <" GLAPP_OSD_VERTEX_SOURCE_STRING ">");
	std::regex  tesscont_include(
        "#include <" GLAPP_OSD_TESS_CONTROL_SOURCE_STRING ">");
	std::regex tesseval_include("#include <" GLAPP_OSD_TESS_EVAL_SOURCE_STRING
								">");

	src = std::regex_replace(shader->GetSource(), common_include,
							 GLSLPatchShaderSource::GetCommonShaderSource());
	src =
		std::regex_replace(src, vertex_include,
						   GLSLPatchShaderSource::GetVertexShaderSource(_type));
	src = std::regex_replace(
		src, tesscont_include,
		GLSLPatchShaderSource::GetTessControlShaderSource(_type));
	src = std::regex_replace(
		src, tesseval_include,
		GLSLPatchShaderSource::GetTessEvalShaderSource(_type));

	return src;
}

#ifndef GLAPP_GL_COMPILE_ERROR_EXCEPTION
const std::string& glslOSDprogram::GetError() const { return err; }
#endif

} // namespace glapp
