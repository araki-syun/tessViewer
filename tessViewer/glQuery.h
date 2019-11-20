#pragma once

#include <GL\glew.h>

class GlQuery {
public:
	GlQuery() = delete;
	GlQuery(GLenum target);
	~GlQuery();

	void   Start();
	void   End();
	GLuint Get();

private:
	GLenum _target{};
	GLuint _id{};
	GLuint _value{};
};
