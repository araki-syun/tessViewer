#pragma once

#include "glm/fwd.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "glapp/glm_conversion.h"
#include "glapp/config.h"

namespace tv {
class Camera {
	template <class BASIC_JSON_TYPE>
	friend void to_json(BASIC_JSON_TYPE& j, const Camera& cam); //NOLINT
	template <class BASIC_JSON_TYPE>
	friend void from_json(const BASIC_JSON_TYPE& j, Camera& cam); //NOLINT

public:
	Camera();
	Camera(glm::vec3 pos, glm::vec3 angle);
	// Camera(glm::vec3 pos, glm::vec3 look, glm::vec3 up);
	Camera(glm::vec3 pos, glm::quat quat);
	Camera(glm::vec3 pos, glm::vec3 angle, float fov, float near, float far);
	// Camera(glm::vec3 pos,
	// 	   glm::vec3 look,
	// 	   glm::vec3 up,
	// 	   float     fov,
	// 	   float     near,
	// 	   float     far);
	Camera(glm::vec3 pos, glm::quat quat, float fov, float near, float far);

	void  Move(glm::vec3 pos);
	void  FpsMove(glm::vec3 move);
	void  Rotate(float x, float y);
	void  Rotate(glm::vec2 xy);
	void  Rotate(glm::vec3 look);
	void  Rotate(glm::quat quat);
	void  RotateMove(glm::vec2 vh);
	float Fov(float fov);
	float Near(float near);
	float Far(float far);
	float Length(float length);
	void  Update(float factor);

	glm::vec3 Position() const;
	glm::vec3 Move() const;
	glm::quat Quaternion() const;
	float     Fov() const;
	float     Near() const;
	float     Far() const;
	float     Length() const;
	glm::mat4 ViewMatrix() const;

	static float MaxFov();
	static float MaxFov(float max);
	static float MinFov();
	static float MinFov(float min);

	static constexpr glm::vec3 front = {0, 0, -1};
	static constexpr glm::vec3 up    = {0, 1, 0};
	static constexpr glm::vec3 right = {1, 0, 0};

protected:
	glm::vec3 _pos;
	glm::quat _quat;
	glm::vec3 _front;
	glm::vec3 _right;
	glm::vec3 _up;
	float     _fov;
	float     _near;
	float     _far;
	float     _length = 10.0f;
	glm::vec3 _move{0};

	static inline float _max_fov = 179.0f;
	static inline float _min_fov = 10.0f;
};
template <class BASIC_JSON_TYPE>
void to_json(BASIC_JSON_TYPE& j, const Camera& cam) { //NOLINT
	j["position"]  = cam._pos;
	j["angle"]     = glm::degrees(glm::eulerAngles(cam._quat));
	j["lookpoint"] = cam._front;
	j["fov"]       = cam._fov;
	j["near"]      = cam._near;
	j["far"]       = cam._far;
	j["length"]    = cam._length;
}
template <class BASIC_JSON_TYPE>
void from_json(const BASIC_JSON_TYPE& j, Camera& cam) { //NOLINT
	auto schema = glapp::Config::Get().Schema("/graphics/camera");
	cam._pos    = j.at("position").get<glm::vec3>();
	cam._quat   = glm::quat(j.at("angle").get<glm::vec3>());
	cam._front  = cam._quat * Camera::front;
	cam._right  = cam._quat * Camera::right;
	cam._up     = cam._quat * Camera::up;
	cam._fov    = std::clamp(j.at("fov").get<float>(),
                          schema.at("property/fov/minimum").get<float>(),
                          schema.at("property/fov/maximum").get<float>());
	float n     = j.at("near").get<float>();
	float f     = j.at("far").get<float>();
	if (n < f) {
		cam._near = n;
		cam._far  = f;
	} else {
		cam._near = schema.at("property/near/default").get<float>();
		cam._far  = schema.at("property/far/default").get<float>();
	}
	cam._length = j.at("length").get<float>();
}
} // namespace tv
