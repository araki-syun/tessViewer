#include "glslprogram.h"

#include <boost\format.hpp>

namespace glapp {
const GLuint glslprogram::GetProgram() const { return _program; }

void glslprogram::SetShaderName(std::initializer_list<std::string> list) {

	for (const std::string& str : list) {
		glslshader* p = glslshader_manager::GetPointer(str);
		switch (p->GetType()) {
		case GL_VERTEX_SHADER: shader_list[shader_type::Vertex] = str; break;
		case GL_FRAGMENT_SHADER:
			shader_list[shader_type::Fragment] = str;
			break;
		case GL_GEOMETRY_SHADER:
			shader_list[shader_type::Geometry] = str;
			break;
		case GL_TESS_CONTROL_SHADER:
			shader_list[shader_type::Tess_Control] = str;
			break;
		case GL_TESS_EVALUATION_SHADER:
			shader_list[shader_type::Tess_Eval] = str;
			break;
		default: break;
		}
	}
}

const GLint glslprogram::GetIndexAttrib(const std::string name) const {
	std::unordered_map<std::string, GLint>::const_iterator it;
	it = attribList.find(name);
	if (it != attribList.cend())
		return it->second;
	return -1;
}

const GLint glslprogram::GetIndexUniform(const std::string name) const {
	std::unordered_map<std::string, GLint>::const_iterator it;
	it = uniformList.find(name);
	if (it != uniformList.cend())
		return it->second;
	return -1;
}

void glslprogram::create() {
	_program = glCreateProgram();

	for (int i = 0; i < 5; ++i) {
		glslshader* g = glslshader_manager::GetPointer(shader_list[i]);
		if (g) {
			GLuint id = shaderCompile(g);
			glAttachShader(_program, id);
			glDeleteShader(id);
		}
	}
	glLinkProgram(_program);

	GLint result;
	glGetProgramiv(_program, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		// サイズを取得
		GLint log_length;
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &log_length);

		// 文字列を取得
		GLsizei             max_length(log_length);
		GLsizei             length;
		std::vector<GLchar> log(max_length);
		glGetProgramInfoLog(_program, max_length, &length, log.data());

#ifdef GLAPP_GL_COMPILE_ERROR_EXCEPTION
		std::string err;
		err.append(log.data(), log.size());

		throw std::runtime_error(
			(boost::format("Shader Program Link ERROR\n%1%") % err).str());
#else
		err.append(log.data(), log.size());
#endif
	}
	setIndexAttrib();
	setIndexUniform();
}

const GLuint glslprogram::shaderCompile(const glslshader* shader) {
	GLuint      id(0);
	const char* str = shader->GetSource().c_str();
	id              = glCreateShader(shader->GetType());
	glShaderSource(id, 1, &str, nullptr);
	glCompileShader(id);

	// 成否を確認
	GLint result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);

	// ログを取得
	if (result == GL_FALSE) {
		// サイズを取得
		GLint log_length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);

		// 文字列を取得
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
}

void glslprogram::setIndexAttrib() {
	GLint max_len, num_attrib;

	glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_len);
	glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &num_attrib);

	std::vector<GLchar> name(max_len);

	for (int i = 0; i < num_attrib; ++i) {
		GLint   size;
		GLsizei len;
		GLenum  type;
		glGetActiveAttrib(_program, i, max_len, &len, &size, &type,
						  name.data());
		GLint loc               = glGetAttribLocation(_program, name.data());
		attribList[name.data()] = loc;
	}
}

void glslprogram::setIndexUniform() {
	GLint max_len, num_uniform;

	glGetProgramiv(_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_len);
	glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &num_uniform);

	std::vector<GLchar> name(max_len);

	for (int i = 0; i < num_uniform; ++i) {
		GLint   size;
		GLsizei len;
		GLenum  type;
		glGetActiveUniform(_program, i, max_len, &len, &size, &type,
						   name.data());
		GLint loc                = glGetUniformLocation(_program, name.data());
		uniformList[name.data()] = loc;
	}
}

} // namespace glapp
