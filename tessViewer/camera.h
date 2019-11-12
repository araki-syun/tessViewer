#pragma once

#include <glm/glm.hpp>

namespace tv {
class Camera {
	glm::vec3 Pos;
	glm::vec3 Angle;
	glm::vec3 LookPoint;
	glm::vec3 Right;
	glm::vec3 Up;
	float     Fov;
	float     Near;
	float     Far;
	glm::vec2 Move;
	bool      FlyMode;
};
} // namespace tv
