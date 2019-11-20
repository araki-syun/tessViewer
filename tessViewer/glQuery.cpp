#include "glQuery.h"

//query::query()
//{
//}

GlQuery::GlQuery(GLenum target) : _target(target) {
	glCreateQueries(target, 1, &_id);
}

GlQuery::~GlQuery() { glDeleteQueries(1, &_id); }

void GlQuery::Start() {
	_value = 0;
	glBeginQuery(_target, _id);
}

void GlQuery::End() { glEndQuery(_target); }

GLuint GlQuery::Get() {
	if (_value == 0) {
		glGetQueryObjectuiv(_id, GL_QUERY_RESULT, &_value);
	}
	return _value;
}
