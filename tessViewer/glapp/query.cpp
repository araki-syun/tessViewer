#include "query.h"

glapp::query::query(void) {}

glapp::query::~query(void) { glDeleteQueries(1, &_id); }

glapp::query::query(GLuint mode) { glGenQueries(1, &_id); }
