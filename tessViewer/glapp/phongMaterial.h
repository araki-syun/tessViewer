#pragma once

#include <glm\glm.hpp>

#include "material.h"

namespace glapp{
	class phongMaterial : material
	{
	public:
		phongMaterial(void);
		~phongMaterial(void);
	private:
		glm::vec3 _diffuse;
		glm::vec3 _specular;
		glm::vec3 _ambient;
	};
}
