#include "shader_program.h"
#include "fmt/core.h"

#include <type_traits>

#include "../log.h"

namespace glapp {
ShaderProgram::ShaderProgram() = default;
ShaderProgram::ShaderProgram(std::initializer_list<std::string> list) {
	SetShaderName(list);
}
ShaderProgram::~ShaderProgram() = default;
ShaderProgram::ShaderProgram(ShaderProgram&& prog) noexcept
	: _program(prog._program)
	, _shader_list(std::move(prog._shader_list))
	, _attrib_list(std::move(prog._attrib_list))
	, _uniform_list(std::move(prog._uniform_list)) {
	prog._program = 0;
}
ShaderProgram& ShaderProgram::operator=(ShaderProgram&& prog) noexcept {
	if (this != &prog) {
		std::swap(_program, prog._program);
		_shader_list  = std::move(prog._shader_list);
		_attrib_list  = std::move(prog._attrib_list);
		_uniform_list = std::move(prog._uniform_list);
	}
	return *this;
}

GLuint ShaderProgram::GetProgram() const { return _program; }
void   ShaderProgram::SetShaderName(std::initializer_list<std::string> list) {

	for (const std::string& str : list) {
		Shader* p = ShaderManager::GetPointer(str);
		switch (p->GetType()) {
		case GL_VERTEX_SHADER: _shader_list[ShaderType::Vertex] = str; break;
		case GL_FRAGMENT_SHADER:
			_shader_list[ShaderType::Fragment] = str;
			break;
		case GL_GEOMETRY_SHADER:
			_shader_list[ShaderType::Geometry] = str;
			break;
		case GL_TESS_CONTROL_SHADER:
			_shader_list[ShaderType::Tess_Control] = str;
			break;
		case GL_TESS_EVALUATION_SHADER:
			_shader_list[ShaderType::Tess_Eval] = str;
			break;
		default: break;
		}
	}
}

GLint ShaderProgram::GetIndexAttrib(const std::string& name) const {
	std::unordered_map<std::string, GLint>::const_iterator it;
	it = _attrib_list.find(name);
	if (it != _attrib_list.cend()) {
		return it->second;
	}
	return -1;
}

GLint ShaderProgram::GetIndexUniform(const std::string& name) const {
	std::unordered_map<std::string, GLint>::const_iterator it;
	it = _uniform_list.find(name);
	if (it != _uniform_list.cend()) {
		return it->second;
	}
	return -1;
}

void ShaderProgram::_Create() {
	_program = glCreateProgram();

	for (int i = 0; i < 5; ++i) {
		Shader* g = ShaderManager::GetPointer(_shader_list.at(i));
		if (g != nullptr) {
			GLuint id = _ShaderCompile(g);
			glAttachShader(_program, id);
			glDeleteShader(id);
		}
	}
	glLinkProgram(_program);

	GLint result;
	glGetProgramiv(_program, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		// �T�C�Y���擾
		GLint log_length;
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &log_length);

		// ��������擾
		GLsizei             max_length(log_length);
		GLsizei             length;
		std::vector<GLchar> log(max_length);
		glGetProgramInfoLog(_program, max_length, &length, log.data());

#ifdef GLAPP_GL_COMPILE_ERROR_EXCEPTION
		std::string err;
		err.append(log.data(), log.size());

		throw tv::GraphicsError(
			tv::LogLevel::Error,
			fmt::format("Shader Program Link ERROR\n{}", err));
#else
		err.append(log.data(), log.size());
#endif
	}
	_SetIndexAttrib();
	_SetIndexUniform();
	tv::Logger::Log<tv::LogLevel::Debug>(tv::InfoType::Graphics, "Shader Program Created");
}

GLuint ShaderProgram::_ShaderCompile(const Shader* shader) {
	GLuint      id(0);
	const char* str = shader->GetSource().c_str();
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

		throw tv::GraphicsError(tv::LogLevel::Error,
								fmt::format("Shader Compile ERROR\n{}", err));
#else
		err.append(log.data(), log.size());
#endif
	}
	return id;
}

void ShaderProgram::_SetIndexAttrib() {
	GLint max_len;
	GLint num_attrib;

	glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_len);
	glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &num_attrib);

	std::vector<GLchar> name(max_len);

	for (int i = 0; i < num_attrib; ++i) {
		GLint   size;
		GLsizei len;
		GLenum  type;
		glGetActiveAttrib(_program, i, max_len, &len, &size, &type,
						  name.data());
		GLint loc                 = glGetAttribLocation(_program, name.data());
		_attrib_list[name.data()] = loc;
	}
}

#ifndef GLAPP_GL_COMPILE_ERROR_EXCEPTION
std::string ShaderProgram::GetError() const { return "error"; }
#endif

void ShaderProgram::_SetIndexUniform() {
	GLint max_len;
	GLint num_uniform;

	glGetProgramiv(_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_len);
	glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &num_uniform);

	std::vector<GLchar> name(max_len);

	for (int i = 0; i < num_uniform; ++i) {
		GLint   size;
		GLsizei len;
		GLenum  type;
		glGetActiveUniform(_program, i, max_len, &len, &size, &type,
						   name.data());
		GLint loc = glGetUniformLocation(_program, name.data());
		_uniform_list[name.data()] = loc;
	}
}

} // namespace glapp
