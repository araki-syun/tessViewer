#include "camera.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
//#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/gtx/quaternion.hpp"

#include "log.h"

namespace tv {
Camera::Camera() : Camera(glm::vec3(0), glm::quat(), 60.0f, 10.0f, 179.0f) {}
Camera::Camera(glm::vec3 pos, glm::vec3 angle)
	: Camera(pos, angle, 60.0f, 10.0f, 179.0f) {}
// Camera::Camera(glm::vec3 pos, glm::vec3 look, glm::vec3 up) {}
Camera::Camera(glm::vec3 pos, glm::quat quat)
	: Camera(pos, quat, 60.0f, 10.0f, 179.0f) {}
Camera::Camera(glm::vec3 pos, glm::vec3 angle, float fov, float near, float far)
	: Camera(pos, glm::quat(angle), fov, near, far) {}
// Camera::Camera(glm::vec3 pos,
// 			   glm::vec3 look,
// 			   glm::vec3 up,
// 			   float     fov,
// 			   float     near,
// 			   float     far) {}
Camera::Camera(glm::vec3 pos, glm::quat quat, float fov, float near, float far)
	: _pos(pos)
	, _quat(quat)
	, _front(_quat * Camera::front)
	, _right(_quat * Camera::right)
	, _up(_quat * Camera::up)
	, _fov(fov)
	, _near(near)
	, _far(far) {
	assert(fov < _max_fov && _min_fov < fov);
	assert(near < far);
	Logger::Log<LogLevel::Debug>(InfoType::Graphics, "Camera Create");
}
void Camera::Move(glm::vec3 pos) {
	_pos = pos;
	Logger::Log<LogLevel::Trace>(
		InfoType::Graphics,
		fmt::format("Camera Move : x:{:3.2f} y:{:3.2f} z:{:3.2f}", _pos.x,
					_pos.y, _pos.z));
}
void Camera::FpsMove(glm::vec3 move) { _move += move; }
void Camera::Rotate(float x, float y) {
	auto h = glm::angleAxis(x, _up);
	auto v = glm::angleAxis(y, _right);
	Rotate(h * v);
}
void Camera::Rotate(glm::vec2 xy) { Rotate(xy.x, xy.y); }
void Camera::Rotate(glm::vec3 look) {
	auto r = glm::rotation(_front, glm::normalize(look - _pos));
	Rotate(r);
}
void Camera::Rotate(glm::quat quat) {
	_quat  = quat * _quat;
	_front = _quat * front;
	_up    = _quat * up;
	_right = _quat * right;
	Logger::Log<LogLevel::Trace>(
		InfoType::Graphics,
		fmt::format("Camera Rotate : w:{:1.3f} x:{:1.3f} y:{:1.3f} z:{:1.3f}",
					_quat.w, _quat.x, _quat.y, _quat.z));
}
void Camera::RotateMove(glm::vec2 vh) {
	auto look = ((_quat * front) * _length) + _pos; //注視点
	Rotate(vh);                                     //カメラ回転
	auto dir = ((_quat * front) * _length);         //回転後カメラ方向
	Move(look - dir); //注視点 - 方向 = 移動位置
}
float Camera::Fov(float fov) {
	assert(fov < _max_fov && _min_fov < fov);
	_fov = fov;
	Logger::Log<LogLevel::Debug>(InfoType::Graphics,
								 fmt::format("Camera Set Fov : {:1.3f}", _fov));
	return _fov;
}
float Camera::Near(float near) {
	_near = near;
	Logger::Log<LogLevel::Debug>(
		InfoType::Graphics, fmt::format("Camera Set Near : {:1.3f}", _near));
	return _near;
}
float Camera::Far(float far) {
	_far = far;
	Logger::Log<LogLevel::Debug>(InfoType::Graphics,
								 fmt::format("Camera Set Far : {:1.3f}", _far));
	return _far;
}
float Camera::Length(float length) {
	_length = length;
	Logger::Log<LogLevel::Debug>(
		InfoType::Graphics,
		fmt::format("Camera Set Length : {:1.3f}", _length));
	return _length;
}
void Camera::Update(float factor) { Move(Position() + _quat * _move * factor); }
glm::vec3 Camera::Position() const { return _pos; }
glm::vec3 Camera::Move() const { return _move; }
glm::quat Camera::Quaternion() const { return _quat; }
float     Camera::Fov() const { return _fov; }
float     Camera::Near() const { return _near; }
float     Camera::Far() const { return _far; }
float     Camera::Length() const { return _length; }
glm::mat4 Camera::ViewMatrix() const {
	return glm::lookAt(_pos, _pos + _front, _up);
}

float Camera::MaxFov() { return _max_fov; }
float Camera::MaxFov(float max) {
	_max_fov = max;
	Logger::Log<LogLevel::Debug>(
		InfoType::Graphics,
		fmt::format("Camera Set MaxFov : {:1.3f}", _max_fov));
	return _max_fov;
}
float Camera::MinFov() { return _min_fov; }
float Camera::MinFov(float min) {
	_min_fov = min;
	Logger::Log<LogLevel::Debug>(
		InfoType::Graphics,
		fmt::format("Camera Set MinFov : {:1.3f}", _min_fov));
	return _min_fov;
}
} // namespace tv
