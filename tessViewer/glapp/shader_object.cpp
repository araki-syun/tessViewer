#include "shader_object.h"

#include <fstream>
#include <vector>
#include <regex>
#include <filesystem>

#include <fmt/format.h>
//#include <opensubdiv\osd\glslPatchShaderSource.h>

#include "glapp_define.h"

namespace glapp {

//std::unordered_map<std::string, Shader> Shader::shader_list;

Shader::Shader() = default;
Shader::Shader(const std::string& filename) {
	std::filesystem::path file_path(filename);

	if (!std::filesystem::exists(file_path)) {
		throw std::runtime_error(
			fmt::format("NOT FOUND Shader File : {}", file_path));
	}

	std::ifstream ifs(filename);
	if (ifs.fail()) {
		throw std::runtime_error(
			fmt::format("LOAD ERROR Shader File : {}", file_path));
	}

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
		throw std::runtime_error(fmt::format("{}", file_path));
	}

	std::string str((std::istreambuf_iterator<char>(ifs)),
					std::istreambuf_iterator<char>());
	_src = str;
}

Shader::~Shader() { glDeleteShader(_id); }

GLuint      Shader::GetShader() const { return _id; }
std::string Shader::GetSource() const { return _src; }
GLuint      Shader::GetType() const { return _type; }
bool        Shader::Empty() const { return _id == 0; }

//	void Shader::Compile(void)
//	{
//		if(_id)
//			return;
//
//		const char* str = _src.c_str();
//		_id = glCreateShader(_type);
//		glShaderSource(_id, 1, &str, NULL);
//		glCompileShader(_id);
//
//		// ���ۂ��m�F
//		GLint result;
//		glGetShaderiv(_id, GL_COMPILE_STATUS, &result);
//
//		// ���O���擾
//		if (result == GL_FALSE) {
//			// �T�C�Y���擾
//			GLint log_length;
//			glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &log_length);
//
//			// ��������擾
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
const std::string& Shader::GetError() const { return err; }
#endif
} // namespace glapp
