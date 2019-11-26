#include "app.h"

#include <fstream>
#include <cstdint>
#include <memory>

#include <opencv2\core.hpp>
#include <opencv2\imgcodecs.hpp>
#include <opencv2\highgui.hpp>

#include <fmt/format.h>

#include <nlohmann/json.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "glapp\config.h"

#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"

#include "version.h"
#include "define.h"
#include "log.h"

using namespace fmt::literals;
using namespace tv;

App::App() {
	auto conf       = glapp::Config::Get();
	auto conf_graph = conf.Relative("/graphics");
	auto conf_osd   = conf_graph.Relative("/osd");
	auto conf_font  = conf.Relative("/ui/font");

	tv::Model::default_patch_type =
		conf.Value<std::string>("/graphics/osd/patch/type") == "GREGORY_BASIS"
			? OpenSubdiv::Far::PatchDescriptor::Type::GREGORY_BASIS
			: OpenSubdiv::Far::PatchDescriptor::Type::REGULAR;

	OpenSubdiv::Far::SetErrorCallback(OsdErrorCallback);
	OpenSubdiv::Far::SetWarningCallback(OsdWarningCallback);

	glfwSetErrorCallback(GlfwErrorCallback);

	std::vector<std::string> extensions{"GL_ARB_direct_state_access",
										"GL_ARB_tessellation_shader"};
	_win = std::make_unique<glapp::Window>("tessViewer " TV_VERSION, 4, 5,
										   extensions);

	int mat_offset           = _material->GetElementSize();
	tv::Model::shader_manage = &(App::_shader_manage);

	tv::Model::default_glsl_info.vert = DEFAULT_VERTEX_SHADER;
	tv::Model::default_glsl_info.frag = DEFAULT_FRAGMENT_SHADER;
	tv::Model::default_glsl_info.geom = DEFAULT_GEOMETRY_SHADER;
	tv::Model::default_glsl_info.CreateUniformBuffer("Transform", 128);
	tv::Model::default_glsl_info.CreateUniformBuffer("Tessellation", 16);
	tv::Model::default_glsl_info.CreateUniformBuffer("LightInfo", 80);
	tv::Model::default_glsl_info.CreateUniformBuffer("MaterialOffset", 16);
	tv::Model::default_glsl_info.GetUniformBuffer("MaterialOffset")
		->Update(&mat_offset);

	tv::Model::default_patch = conf_osd.Value<int>("/patch/level");
	tv::Model::max_patch = conf_osd.Schema("/patch/level/maximum").get<int>();

	_draw_string = tv::GlslStringDraw::GetInstance();
	_draw_string->Initialize(conf_font.Value<int>("size"),
							 conf_font.Value<std::string>("file"));
	_draw_string->SetWindowSize(conf.Value<int>("/window/resolution/width"),
								conf.Value<int>("/window/resolution/height"));

	// コマンドライン引数または設定で指定されたsdmjファイルから読み込む
	auto          sdmjname = conf_graph.Value<std::string>("Model");
	std::ifstream sdmj(sdmjname);
	if (!sdmj) {
		throw AppError(LogLevel::Error,
					   "File Open Error : {}"_format(sdmjname));
	}

	nlohmann::json j;
	sdmj >> j;
	auto& objects = j["objects"];
	_material     = std::make_shared<tv::Material>(j["materials"]);
	_models.reserve(objects.size());
	for (auto& o : objects) {
		_models.emplace_back(o, _material);
	}

	_window_size = glm::ivec2(conf.Value<int>("/window/resolution/width"),
							  conf.Value<int>("/window/resolution/height"));

	_camera.MaxFov(conf_graph.Schema("/camera/fov/maximum").get<float>());
	_camera.MinFov(conf_graph.Schema("/camera/fov/minimum").get<float>());
	_camera = conf_graph.Value<tv::Camera>("camera");

	_tess_fact     = conf_osd.Value<int>("/tessellation/level");
	_max_tess_fact = conf_osd.Schema("/tessellation/level/maximum").get<int>();

	_update_view();
	_update_projection();

	auto light_color = conf_osd.Value<glm::vec3>("/light/position");

	_light = {conf_osd.Value<glm::vec3>("/light/position"),
			  conf_osd.Value<glm::vec3>("/light/position"), light_color,
			  light_color, light_color};

	_tess.Update(std::min(_max_tess_fact, _tess_fact));

	cv::Mat default_texture =
		cv::imread("{}{}"_format(TEXTURE, "default_texture.png"));
	cv::Mat flip_texture(
		cv::Size(default_texture.size[0], default_texture.size[1]), CV_8UC3);
	cv::flip(default_texture, flip_texture, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glCreateTextures(GL_TEXTURE_2D, 1, &_default_diffuse_texture);
	glTextureImage2DEXT(_default_diffuse_texture, GL_TEXTURE_2D, 0,
						GL_COMPRESSED_RGB, flip_texture.cols, flip_texture.rows,
						0, GL_BGR, GL_UNSIGNED_BYTE, flip_texture.data);
	glTextureParameteri(_default_diffuse_texture, GL_TEXTURE_MAG_FILTER,
						GL_LINEAR);
	glTextureParameteri(_default_diffuse_texture, GL_TEXTURE_MIN_FILTER,
						GL_LINEAR);
	tv::Model::default_texture = _default_diffuse_texture;
}
App::~App() {
	if (_default_diffuse_texture != 0u) {
		glDeleteTextures(1, &_default_diffuse_texture);
	}
}

void App::Run() {
	GLFWwindow* w = _win->GetWin();

	glfwSetWindowUserPointer(w, this);
	glfwSetKeyCallback(w, KeyDefaultCallback);
	glfwSetMouseButtonCallback(w, MouseButtonDfaultCallback);
	glfwSetScrollCallback(w, MouseScrollLengthCallback);
	glfwSetWindowSizeCallback(w, WindowResizeCallback);

	std::string patch_type(
		tv::Model::default_patch_type ==
				OpenSubdiv::Far::PatchDescriptor::Type::REGULAR
			? "REGULAR"
			: "GREGORY_BASIS");

	_query = std::make_unique<GlQuery>(GL_PRIMITIVES_GENERATED);
	//draw_string_query.reset(new glQuery(GL_PRIMITIVES_GENERATED));
	glfwSetTime(0.0);
	_mainloop = true;
	while (_mainloop && (glfwWindowShouldClose(w) == 0)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		tv::Model::draw_call = 0;

		_update_ubo();

		_query->Start();
		for (tv::Model& model : _models) {
			model.Draw();
		}
		_frametime = (float)glfwGetTime();
		glfwSetTime(0.0);
		_query->End();

		//draw_string_query->Start();
		auto format = "{:-12s} : {:3.2f}\n"_format;
		_draw_string->Set(5, 0, format("FPS", 1.f / _frametime));
		_draw_string->Set(5, 18, format("Tess Level", _tess_fact));
		_draw_string->Set(5, 36, format("Primitive", _query->Get()));
		_draw_string->Set(5, 54, format("Patch Type", patch_type));
		_draw_string->Set(5, 72,
						  format("Patch Level", tv::Model::default_patch));
		_draw_string->Set(5, 90, format("Draw Call", tv::Model::draw_call));
		_draw_string->Draw();
		//draw_string_query->End();

		glfwPollEvents();

		glm::dvec2 temp;
		glfwGetCursorPos(w, &temp.x, &temp.y);
		_previous_mouse_pos = temp;

		glfwSwapBuffers(w);
		glFlush();
	}
}

void App::OsdErrorCallback(OpenSubdiv::Far::ErrorType err,
						   const char*                message) {
	throw GraphicsError(LogLevel::Error,
						"{} : {} {}"_format("OpenSubdiv Error Type",
											std::to_string(err), message));
}
void App::OsdWarningCallback(const char* message) {
	Logger::Log(LogLevel::Warning, InfoType::Graphics, message);
}
void App::GlfwErrorCallback(int code, const char* message) {
	throw GraphicsError(LogLevel::Error,
						"{:12s} : {}\n{}\n"_format(
							"GLFW ERROR Code", std::to_string(code), message));
}
void App::KeyDefaultCallback(
	GLFWwindow* window, int key, int scancode, int action, int mods) {
	App* a = static_cast<App*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE: a->_mainloop = false; break;
		case GLFW_KEY_F:
			//glfwSetCursorPosCallback(a->win->GetWin(), nullptr);
			//glfwSetKeyCallback(a->win->GetWin(), KeyFlyModeCallback);
			//glfwSetMouseButtonCallback(a->win->GetWin(), MouseButtonFlayModeCallback);
			//a->_fly_mode = true;
			//std::cout << "Fly Mode" << std::endl;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			glfwSetScrollCallback(a->_win->GetWin(), MouseScrollFovCallback);
			break;
		case GLFW_KEY_UP:
			a->_tess_fact = glm::min(a->_tess_fact + 1, a->_max_tess_fact);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_DOWN:
			a->_tess_fact = glm::max(a->_tess_fact - 1, 0);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_SPACE: {
			auto q      = a->_camera.Quaternion();
			auto format = "{:-12s} : {:3.2f}\n"_format;
			Logger::Log(
				LogLevel::Debug, InfoType::Control,
				format(
					"Pos", glm::to_string(a->_camera.Position()),
					format("Angle",
						   glm::to_string(glm::degrees(glm::eulerAngles(q)))),
					format("LookPoint", glm::to_string(q * tv::Camera::front)),
					format("Right", glm::to_string(q * tv::Camera::right)),
					format("Up", glm::to_string(q * tv::Camera::up))));
		} break;
		default: break;
		}
	} else if (action == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_LEFT_SHIFT:
			glfwSetScrollCallback(a->_win->GetWin(), MouseScrollLengthCallback);
			break;
		default: break;
		}
	}
}
void App::KeyFlyModeCallback(
	GLFWwindow* window, int key, int scancode, int action, int mods) {
	App* a = static_cast<App*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE: a->_mainloop = false; break;
		case GLFW_KEY_F:
			glfwSetCursorPosCallback(a->_win->GetWin(), nullptr);
			glfwSetKeyCallback(a->_win->GetWin(), KeyDefaultCallback);
			glfwSetMouseButtonCallback(a->_win->GetWin(),
									   MouseButtonDfaultCallback);
			a->_fly_mode = false;
			DragCameraRotate(window, 0.0, 0.0);
			Logger::Log(LogLevel::Debug, InfoType::Control, "Normal Mode");
			break;
		case GLFW_KEY_UP:
			a->_tess_fact = glm::min(a->_tess_fact + 1, a->_max_tess_fact);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_DOWN:
			a->_tess_fact = glm::max(a->_tess_fact - 1, 0);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_W: a->_camera.FpsMove(tv::Camera::front); break;
		case GLFW_KEY_S: a->_camera.FpsMove(-tv::Camera::front); break;
		case GLFW_KEY_D: a->_camera.FpsMove(tv::Camera::right); break;
		case GLFW_KEY_A: a->_camera.FpsMove(-tv::Camera::right); break;
		case GLFW_KEY_SPACE: {
			auto q      = a->_camera.Quaternion();
			auto format = "{:-12s} : {:3.2f}\n"_format;
			Logger::Log(
				LogLevel::Debug, InfoType::Control,
				format(
					"Pos", glm::to_string(a->_camera.Position()),
					format("Angle",
						   glm::to_string(glm::degrees(glm::eulerAngles(q)))),
					format("LookPoint", glm::to_string(q * tv::Camera::front)),
					format("Right", glm::to_string(q * tv::Camera::right)),
					format("Up", glm::to_string(q * tv::Camera::up))));
		} break;
		default: break;
		}
	}
	if (action == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_W: a->_camera.FpsMove(-tv::Camera::front); break;
		case GLFW_KEY_S: a->_camera.FpsMove(tv::Camera::front); break;
		case GLFW_KEY_D: a->_camera.FpsMove(-tv::Camera::right); break;
		case GLFW_KEY_A: a->_camera.FpsMove(tv::Camera::right); break;
		default: break;
		}
	}
	a->_update_view();
}
void App::DragCameraRotate(GLFWwindow* window, double x, double y) {
	App* a = static_cast<App*>(glfwGetWindowUserPointer(window));

	auto current = glm::vec2(x, y);
	auto move    = (current - a->_previous_mouse_pos) * a->_frametime;

	a->_camera.RotateMove(move);

	a->_previous_mouse_pos = current;
	a->_update_view();
}
void App::DragCameraTranslation(GLFWwindow* window, double x, double y) {
	App* a = static_cast<App*>(glfwGetWindowUserPointer(window));

	auto current = glm::vec2(x, y);
	auto move =
		glm::vec3((current - a->_previous_mouse_pos) * a->_frametime, 0);

	a->_camera.FpsMove(move);

	a->_previous_mouse_pos = current;
	a->_update_view();
}
void App::DragCameraFlyMode(GLFWwindow* window, double x, double y) {
	App* a = static_cast<App*>(glfwGetWindowUserPointer(window));

	auto current = glm::vec2(x, y);
	auto move    = (current - a->_previous_mouse_pos) * a->_frametime;

	a->_camera.Rotate(move);

	a->_previous_mouse_pos = current;
	a->_update_view();
}
void App::MouseButtonDfaultCallback(GLFWwindow* window,
									int         button,
									int         action,
									int         mods) {
	App* a = static_cast<App*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_MIDDLE:
			if (mods == GLFW_MOD_SHIFT) {
				glfwSetCursorPosCallback(a->_win->GetWin(),
										 DragCameraTranslation);
				Logger::Log(LogLevel::Debug, InfoType::Control,
							"Camera Translation");
			} else {
				glfwSetCursorPosCallback(a->_win->GetWin(), DragCameraRotate);
				Logger::Log(LogLevel::Debug, InfoType::Control,
							"Camera Rotate");
			}
			break;
		default: break;
		}
	} else if (action == GLFW_RELEASE) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_MIDDLE:
			glfwSetCursorPosCallback(a->_win->GetWin(), nullptr);
			break;
		default: break;
		}
	}
}
void App::MouseButtonFlayModeCallback(GLFWwindow* window,
									  int         button,
									  int         action,
									  int         mods) {
	App* a = static_cast<App*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_MIDDLE:
			if (mods == GLFW_MOD_SHIFT) {
				glfwSetCursorPosCallback(a->_win->GetWin(),
										 DragCameraTranslation);
				Logger::Log(LogLevel::Debug, InfoType::Control,
							"Camera Translation");
			} else {
				glfwSetCursorPosCallback(a->_win->GetWin(), DragCameraFlyMode);
				Logger::Log(LogLevel::Debug, InfoType::Control,
							"Camera Rotate");
			}
			break;
		default: break;
		}
	} else if (action == GLFW_RELEASE) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_MIDDLE:
			glfwSetCursorPosCallback(a->_win->GetWin(), nullptr);
			break;
		default: break;
		}
	}
}
void App::MouseScrollFovCallback(GLFWwindow* window, double up, double down) {
	App* a = static_cast<App*>(glfwGetWindowUserPointer(window));

	using namespace tv;
	auto fov = a->_camera.Fov() + (float)(up - down);
	a->_camera.Fov(glm::clamp(fov, Camera::MinFov(), Camera::MaxFov()));
	a->_update_projection();
	Logger::Log(LogLevel::Debug, InfoType::Control,
				"{:-12s} : {:3.2f}\n"_format("fov : ", a->_camera.Fov()));
}
void App::MouseScrollLengthCallback(GLFWwindow* window,
									double      up,
									double      down) {
	App* a = static_cast<App*>(glfwGetWindowUserPointer(window));

	a->_camera.Length((a->_camera.Length() * 0.1f) * (float)(down - up));

	a->_update_view();
}
void App::WindowResizeCallback(GLFWwindow* window, int x, int y) {
	App* a = static_cast<App*>(glfwGetWindowUserPointer(window));

	glViewport(0, 0, x, y);
	a->_window_size = glm::ivec2(x, y);
	a->_update_projection();
	a->_draw_string->SetWindowSize(x, y);
}

void App::_update_view() { _transform.view = _camera.ViewMatrix(); }
void App::_update_projection() {
	_transform.projection =
		glm::perspective(glm::radians(_camera.Fov()),
						 (float)_window_size.x / (float)_window_size.y,
						 _camera.Near(), _camera.Far());
}
void App::_update_ubo() {
	_tess.Update(_tess_fact);
	tv::Model::default_glsl_info.GetUniformBuffer("Transform")
		->Update(&_transform);
	tv::Model::default_glsl_info.GetUniformBuffer("LightInfo")->Update(&_light);
	tv::Model::default_glsl_info.GetUniformBuffer("Tessellation")
		->Update(&_tess);
	//shader_default->BindUniformBlock();
}
