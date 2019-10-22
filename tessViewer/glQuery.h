#pragma once

#include <GL\glew.h>

class glQuery {
public:
	glQuery() = delete;
	glQuery(GLenum target);
	~glQuery();

	void   Start();
	void   End();
	GLuint Get();

private:
	GLenum target;
	GLuint id;
	GLuint value;
};
