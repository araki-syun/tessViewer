#include "material.h"

namespace glapp {
material::material(void) {}

material::~material(void) {}

const glslprogram* material::GetGLSLProgram() const { return program.get(); }
} // namespace glapp
