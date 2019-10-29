#include "material.h"

namespace glapp {
material::material(void) {}

material::~material(void) {}

const glShaderProgram* material::GetGLSLProgram() const { return program.get(); }
} // namespace glapp
