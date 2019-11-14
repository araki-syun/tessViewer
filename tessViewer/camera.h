#pragma once

#include "glm/fwd.hpp"
#include "nlohmann/json_fwd.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "glapp/glm_conversion.h"
#include "glapp/config.h"

namespace tv {
class Camera {
	// template <class BasicJsonType>
	friend void to_json(nlohmann::json& j, const Camera& cam);
	// template <class BasicJsonType>
	friend void from_json(const nlohmann::json& j, Camera& cam);

public:
	Camera();

protected:
	glm::vec3 _pos;
	glm::quat _quat;
	glm::vec3 _lookpoint;
	glm::vec3 _right;
	glm::vec3 _up;
	float     _fov;
	float     _near;
	float     _far;

	static float _max_fov;
	static float _min_fov;

	static constexpr glm::vec3 front = {0, 0, -1};
	static constexpr glm::vec3 up    = {1, 0, 0};
	static constexpr glm::vec3 right = {0, 1, 0};
};
} // namespace tv
