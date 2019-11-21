#pragma once

#include <string>

#include <GL\glew.h>

#include "opensubdiv\osd\opengl.h"

namespace tv {
class GlShaderVariable {
public:
	GlShaderVariable();
	virtual ~GlShaderVariable() = 0;

	GLuint             GetIndex() const;
	const std::string& GetVariableName() const;

protected:
	GLint       _variable_location;
	std::string _variable_name;
};

class GlShaderAttribute : public GlShaderVariable {
public:
	GlShaderAttribute(GLuint program, const char* name);
	~GlShaderAttribute() override;

	void Set(GLuint program, const char* name);
};

class GlShaderUniform : public GlShaderVariable {
public:
	GlShaderUniform(GLuint program, const char* name);
	~GlShaderUniform() override;

	void Set(GLuint program, const char* name);
};

class GlShaderUniformBlock : public GlShaderVariable {
public:
	GlShaderUniformBlock(GLuint                        program,
						 /*int buf_size,*/ const char* name);
	~GlShaderUniformBlock() override;

	void Set(GLuint program, const char* name);

	/*private:
		int _buf_size;*/
};

class GlShaderUniformBuffer {
public:
	GlShaderUniformBuffer(
		/*GLuint program,*/ int buf_size /*, const char* name*/);
	~GlShaderUniformBuffer();

	void   Update(const void* val) const;
	void   Set(/*GLuint program, const char* name*/);
	GLuint GetBuffer() const;
	GLuint GetBinding() const;

private:
	static GLuint _bindindex;
	GLuint        _binding;
	GLuint        _buffer;
	int           _buf_size;
};

} // namespace tv
