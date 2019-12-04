#include "glShaderVariable.h"

namespace tv {
GlShaderVariable::GlShaderVariable() : _variable_location(-1) {}
GlShaderVariable::~GlShaderVariable() = default;
GLuint GlShaderVariable::GetIndex() const { return _variable_location; }
const std::string& GlShaderVariable::GetVariableName() const {
	return _variable_name;
}

GlShaderAttribute::GlShaderAttribute(GLuint program, const char* name) {
	this->Set(program, name);
}
GlShaderAttribute::~GlShaderAttribute() = default;
void GlShaderAttribute::Set(GLuint program, const char* name) {
	_variable_location = glGetAttribLocation(program, name);
	if (_variable_location < 0) {
		return;
	}
	_variable_name = name;
}

GlShaderUniform::GlShaderUniform(GLuint program, const char* name) {
	this->Set(program, name);
}
GlShaderUniform::~GlShaderUniform() = default;
void GlShaderUniform::Set(GLuint program, const char* name) {
	_variable_location = glGetUniformLocation(program, name);
	if (_variable_location < 0) {
		return;
	}
	_variable_name = name;
}

GlShaderUniformBlock::GlShaderUniformBlock(
	GLuint program, /*int buf_size,*/ const char* name) /*:
		_buf_size(buf_size)*/
{
	this->Set(program, name);
}
GlShaderUniformBlock::~GlShaderUniformBlock() = default;
void GlShaderUniformBlock::Set(GLuint program, const char* name) {
	_variable_location = glGetUniformBlockIndex(program, name);
	if (_variable_location < 0) {
		return;
	}
	_variable_name = name;
}

GlShaderUniformBuffer::GlShaderUniformBuffer(
	/*GLuint program,*/ int buf_size /*, const char* name*/)
	: _binding(_bindindex++), _buffer(0), _buf_size(buf_size) {
	this->Set(/*program, name*/);
}
GlShaderUniformBuffer::~GlShaderUniformBuffer() {
	if (_buffer != 0u) {
		glDeleteBuffers(1, &_buffer);
	}
	--_bindindex;
}

void GlShaderUniformBuffer::Update(const void* val) const {
	glNamedBufferSubData(_buffer, 0, _buf_size, val);

	glBindBufferBase(GL_UNIFORM_BUFFER, _binding, _buffer);
}
void GlShaderUniformBuffer::Set() {
	if (_buffer != 0u) {
		glDeleteBuffers(1, &_buffer);
	}
	glCreateBuffers(1, &_buffer);
	glNamedBufferData(_buffer, _buf_size, nullptr, GL_DYNAMIC_DRAW);
}
GLuint GlShaderUniformBuffer::GetBuffer() const { return _buffer; }
GLuint GlShaderUniformBuffer::GetBinding() const { return _binding; }

GLuint GlShaderUniformBuffer::_bindindex = 0;

} // namespace tv
