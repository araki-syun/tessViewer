#pragma once

#include <memory>
#include "glslprogram.h"

namespace glapp{
	class material
	{
	public:
		material(void);
		virtual ~material(void);

		const glslprogram* GetGLSLProgram() const;

		virtual void Uniform() const = 0;

	private:
		std::shared_ptr<glslprogram> program;
	};
}

