#pragma once

#include <string>

#include <GL\glew.h>

#include "opensubdiv\osd\opengl.h"

namespace tv {
class glShaderVariable {
public:
	glShaderVariable();
	virtual ~glShaderVariable() = 0;

	const GLuint       GetIndex() const;
	const std::string& GetVariableName() const;

protected:
	GLint       _variable_location;
	std::string _variable_name;
};

class glShaderAttribute : public glShaderVariable {
public:
	glShaderAttribute(GLuint program, const char* name);
	~glShaderAttribute();

	void Set(GLuint program, const char* name);
};

class glShaderUniform : public glShaderVariable {
public:
	glShaderUniform(GLuint program, const char* name);
	~glShaderUniform();

	void Set(GLuint program, const char* name);
};

class glShaderUniformBlock : public glShaderVariable {
public:
	glShaderUniformBlock(GLuint                        program,
						 /*int buf_size,*/ const char* name);
	~glShaderUniformBlock();

	void Set(GLuint program, const char* name);

	/*private:
		int _buf_size;*/
};

class glShaderUniformBuffer {
public:
	glShaderUniformBuffer(
		/*GLuint program,*/ int buf_size /*, const char* name*/);
	~glShaderUniformBuffer();

	void   Update(const void* val) const;
	void   Set(/*GLuint program, const char* name*/);
	GLuint GetBuffer() const;
	GLuint GetBinding() const;

private:
	static GLuint _bindindex;
	GLuint        _binding;
	GLuint        _buffer;
	int           _bufSize;
};

} // namespace tv
