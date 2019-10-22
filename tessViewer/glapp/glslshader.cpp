#include "glslshader.h"

#include <fstream>
#include <vector>
#include <regex>

#include <boost\format.hpp>
#include <boost\filesystem.hpp>
#include <boost\filesystem\path.hpp>

//#include <opensubdiv\osd\glslPatchShaderSource.h>

#include "glapp_define.h"

namespace glapp {

//std::unordered_map<std::string, glslshader> glslshader::shader_list;

glslshader::glslshader(void) : _id(0) {}
glslshader::glslshader(const std::string& filename) : _id(0) {
	boost::filesystem::path file_path(filename);
	if (!boost::filesystem::exists(file_path))
		throw std::runtime_error(
			(boost::format("NOT FOUND Shader File : %1%") % file_path).str());

	std::ifstream ifs(filename);
	if (ifs.fail())
		throw std::runtime_error(
			(boost::format("LOAD ERROR Shader File : %1%") % file_path).str());

	std::string ext = file_path.extension().string();

	if (ext == ".vert") {
		_type = GL_VERTEX_SHADER;
	} else if (ext == ".frag") {
		_type = GL_FRAGMENT_SHADER;
	} else if (ext == ".geom") {
		_type = GL_GEOMETRY_SHADER;
	} else if (ext == ".tess_control") {
		_type = GL_TESS_CONTROL_SHADER;
	} else if (ext == ".tess_eval") {
		_type = GL_TESS_EVALUATION_SHADER;
	} else if (ext == ".glsl") {
		_type = 0;
	} else {
		throw std::runtime_error(
			(boost::format("") % file_path.string()).str());
	}

	std::string str((std::istreambuf_iterator<char>(ifs)),
					std::istreambuf_iterator<char>());
	_src = str;
}

glslshader::~glslshader(void) { glDeleteShader(_id); }

const GLuint glslshader::GetShader() const { return _id; }

const std::string& glslshader::GetSource() const { return _src; }

const GLuint glslshader::GetType() const { return _type; }

//	void glslshader::Compile(void)
//	{
//		if(_id)
//			return;
//
//		const char* str = _src.c_str();
//		_id = glCreateShader(_type);
//		glShaderSource(_id, 1, &str, NULL);
//		glCompileShader(_id);
//
//		// 成否を確認
//		GLint result;
//		glGetShaderiv(_id, GL_COMPILE_STATUS, &result);
//
//		// ログを取得
//		if (result == GL_FALSE) {
//			// サイズを取得
//			GLint log_length;
//			glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &log_length);
//
//			// 文字列を取得
//			GLsizei max_length;
//			GLsizei length;
//			std::vector<GLchar> log(max_length);
//			glGetShaderInfoLog(_id, max_length, &length, log.data());
//
//#ifdef GLAPP_GL_COMPILE_ERROR_EXCEPTION
//			std::string err;
//			err.append(log.data(), log.size());
//
//			throw std::runtime_error((boost::format("Shader Compile ERROR\n%1%") % err).str());
//#else
//			err.append(log.data(), log.size());
//#endif
//		}
//	}

#ifndef GLAPP_GL_COMPILE_ERROR_EXCEPTION
const std::string& glslshader::GetError() const { return err; }
#endif
} // namespace glapp
