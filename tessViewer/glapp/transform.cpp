#include "transform.h"

transform::transform(void) {}

transform::~transform(void) {}

const float* transform::data(void) const { return &_translate.x; }
