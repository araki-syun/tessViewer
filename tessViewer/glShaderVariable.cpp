#include "glShaderVariable.h"

namespace tv {
glShaderVariable::glShaderVariable() : _variable_location(-1) {}
glShaderVariable::~glShaderVariable() = default;
GLuint glShaderVariable::GetIndex() const { return _variable_location; }
const std::string& glShaderVariable::GetVariableName() const {
	return _variable_name;
}

glShaderAttribute::glShaderAttribute(GLuint program, const char* name) {
	this->Set(program, name);
}
glShaderAttribute::~glShaderAttribute() = default;
void glShaderAttribute::Set(GLuint program, const char* name) {
	_variable_location = glGetAttribLocation(program, name);
	if (_variable_location < 0) {
		return;
	}
	_variable_name = name;
}

glShaderUniform::glShaderUniform(GLuint program, const char* name) {
	this->Set(program, name);
}
glShaderUniform::~glShaderUniform() = default;
void glShaderUniform::Set(GLuint program, const char* name) {
	_variable_location = glGetUniformLocation(program, name);
	if (_variable_location < 0) {
		return;
	}
	_variable_name = name;
}

glShaderUniformBlock::glShaderUniformBlock(
	GLuint program, /*int buf_size,*/ const char* name) /*:
		_buf_size(buf_size)*/
{
	this->Set(program, name);
}
glShaderUniformBlock::~glShaderUniformBlock() = default;
void glShaderUniformBlock::Set(GLuint program, const char* name) {
	_variable_location = glGetUniformBlockIndex(program, name);
	if (_variable_location < 0) {
		return;
	}
	_variable_name = name;
}

glShaderUniformBuffer::glShaderUniformBuffer(
	/*GLuint program,*/ int buf_size /*, const char* name*/)
	: _binding(_bindindex++), _buffer(0), _bufSize(buf_size) {
	this->Set(/*program, name*/);
}
glShaderUniformBuffer::~glShaderUniformBuffer() {
	if (_buffer != 0u) {
		glDeleteBuffers(1, &_buffer);
	}
	--_bindindex;
}

void glShaderUniformBuffer::Update(const void* val) const {
	glNamedBufferSubData(_buffer, 0, _bufSize, val);

	glBindBufferBase(GL_UNIFORM_BUFFER, _binding, _buffer);
}
void glShaderUniformBuffer::Set() {
	if (_buffer != 0u) {
		glDeleteBuffers(1, &_buffer);
	}
	glCreateBuffers(1, &_buffer);
	glNamedBufferData(_buffer, _bufSize, nullptr, GL_DYNAMIC_DRAW);
}
GLuint glShaderUniformBuffer::GetBuffer() const { return _buffer; }
GLuint glShaderUniformBuffer::GetBinding() const { return _binding; }

GLuint glShaderUniformBuffer::_bindindex = 0;

} // namespace tv
