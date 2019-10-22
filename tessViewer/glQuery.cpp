#include "glQuery.h"

//query::query()
//{
//}

glQuery::glQuery(GLenum target) : target(target) {
	glCreateQueries(target, 1, &id);
}

glQuery::~glQuery() { glDeleteQueries(1, &id); }

void glQuery::Start() {
	value = 0;
	glBeginQuery(target, id);
}

void glQuery::End() { glEndQuery(target); }

GLuint glQuery::Get() {
	if (value == 0)
		glGetQueryObjectuiv(id, GL_QUERY_RESULT, &value);
	return value;
}
