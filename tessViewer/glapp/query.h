#pragma once

#include <GL\glew.h>

namespace glapp{
	class query
	{
	public:
		query(void);
		~query(void);
		query(GLuint mode);
	private:
		GLuint _id;
	};
}

