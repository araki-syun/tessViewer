#pragma once

#include <memory>
#include "glShaderProgram.h"

namespace glapp {
class material {
public:
	material(void);
	virtual ~material(void);

	const glShaderProgram* GetGLSLProgram() const;

	virtual void Uniform() const = 0;

private:
	std::shared_ptr<glShaderProgram> program;
};
} // namespace glapp
