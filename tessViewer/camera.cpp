#include "camera.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
//#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/gtx/quaternion.hpp"

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
}
void Camera::Move(glm::vec3 pos) { _pos = pos; }
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
}
void Camera::RotateMove(glm::vec2 vh) {
	auto look = ((_quat * front) * _length) + _pos; //注視点
	Rotate(vh);                                     //カメラ回転
	auto dir = ((_quat * front) * _length);         //回転後カメラ方向
	Move(look - dir); //注視点 - 方向 = 移動位置
}
float Camera::Fov(float fov) {
	assert(fov < _max_fov && _min_fov < fov);
	return _fov = fov;
}
float     Camera::Near(float near) { return _near = near; }
float     Camera::Far(float far) { return _far = far; }
float     Camera::Length(float length) { return _length = length; }
void      Camera::Update(float factor) { _pos += _quat * _move * factor; }
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
float Camera::MaxFov(float max) { return _max_fov = max; }
float Camera::MinFov() { return _min_fov; }
float Camera::MinFov(float min) { return _min_fov = min; }
} // namespace tv
