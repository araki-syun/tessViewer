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
template <class BasicJsonType>
void to_json(BasicJsonType& j, const Camera& cam) {
	j["position"]  = cam._pos;
	j["angle"]     = glm::degrees(glm::eulerAngles(cam._quat));
	j["lookpoint"] = cam._lookpoint;
	j["fov"]       = cam._fov;
	j["near"]      = cam._near;
	j["far"]       = cam._far;
}
template <class BasicJsonType>
void from_json(const BasicJsonType& j, Camera& cam) {
	auto schema    = glapp::Config::Get().Schema("/graphics/camera");
	cam._pos       = j.at("position").get<glm::vec3>();
	cam._quat      = glm::quat(j.at("angle").get<glm::vec3>());
	cam._lookpoint = cam._quat * Camera::front;
	cam._right     = cam._quat * Camera::right;
	cam._up        = cam._quat * Camera::up;
	cam._fov       = std::clamp(j.at("fov").get<float>(),
                          schema.at("property/fov/minimum").get<float>(),
                          schema.at("property/fov/maximum").get<float>());
	if (auto near = j.at("near").get<float>(), far = j.at("far").get<float>();
		near < far) {
		cam._near = j.at("near").get<float>();
		cam._far  = j.at("far").get<float>();
	} else {
		cam._near = schema.at("property/near/default").get<float>();
		cam._far  = schema.at("property/far/default").get<float>();
	}
}
} // namespace tv
