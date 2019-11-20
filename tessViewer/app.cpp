#include "app.h"

#include <fstream>
#include <cstdint>
#include <memory>

#include <opencv2\core.hpp>
#include <opencv2\imgcodecs.hpp>
#include <opencv2\highgui.hpp>

#include <nlohmann/json.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <vector>

#include "glapp\config.h"
#include "glapp\glapp_define.h"

#include "glm/fwd.hpp"
#include "version.h"

app::app() {
	auto conf       = glapp::Config::Get();
	auto conf_graph = conf.Relative("/graphics");
	auto conf_osd   = conf_graph.Relative("/osd");
	auto conf_font  = conf.Relative("/ui/font");

	tv::model::default_patch_type =
		conf.Value<std::string>("/graphics/osd/patch/type") == "GREGORY_BASIS"
			? OpenSubdiv::Far::PatchDescriptor::Type::GREGORY_BASIS
			: OpenSubdiv::Far::PatchDescriptor::Type::REGULAR;

	OpenSubdiv::Far::SetErrorCallback(osdErrorCallback);
	OpenSubdiv::Far::SetWarningCallback(osdWarningCallback);

	glfwSetErrorCallback(glfwErrorCallback);

	std::vector<std::string> extensions{"GL_ARB_direct_state_access",
										"GL_ARB_tessellation_shader"};
	win = std::make_unique<glapp::Window>("tessViewer " TV_VERSION, 4, 5,
										  extensions);

	int mat_offset           = material->GetElementSize();
	tv::model::shader_manage = &(app::shader_manage);

	tv::model::default_glsl_info.vert = DEFAULT_VERTEX_SHADER;
	tv::model::default_glsl_info.frag = DEFAULT_FRAGMENT_SHADER;
	tv::model::default_glsl_info.geom = DEFAULT_GEOMETRY_SHADER;
	tv::model::default_glsl_info.CreateUniformBuffer("Transform", 128);
	tv::model::default_glsl_info.CreateUniformBuffer("Tessellation", 16);
	tv::model::default_glsl_info.CreateUniformBuffer("LightInfo", 80);
	tv::model::default_glsl_info.CreateUniformBuffer("MaterialOffset", 16);
	tv::model::default_glsl_info.GetUniformBuffer("MaterialOffset")
		->Update(&mat_offset);

	tv::model::default_patch = conf_osd.Value<int>("/patch/level");
	tv::model::max_patch = conf_osd.Schema("/patch/level/maximum").get<int>();

	draw_string = tv::glslStringDraw::getInstance();
	draw_string->Initialize(conf_font.Value<int>("size"),
							conf_font.Value<std::string>("file"));
	draw_string->SetWindowSize(conf.Value<int>("/window/resolution/width"),
							   conf.Value<int>("/window/resolution/height"));

	// コマンドライン引数または設定で指定されたsdmjファイルから読み込む
	std::ifstream sdmj(conf_graph.Value<std::string>("model"));
	if (!sdmj) {
		throw std::runtime_error("File Open Error\n");
	}

	nlohmann::json j;
	sdmj >> j;
	auto& objects = j["objects"];
	material      = std::make_shared<tv::material>(j["materials"]);
	models.reserve(objects.size());
	for (auto& o : objects) {
		models.emplace_back(o, material);
	}

	window_size = glm::ivec2(conf.Value<int>("/window/resolution/width"),
							 conf.Value<int>("/window/resolution/height"));

	_camera.MaxFov(conf_graph.Schema("/camera/fov/maximum").get<float>());
	_camera.MinFov(conf_graph.Schema("/camera/fov/minimum").get<float>());
	_camera = conf_graph.Value<tv::Camera>("camera");

	tess_fact     = conf_osd.Value<int>("/tessellation/level");
	max_tess_fact = conf_osd.Schema("/tessellation/level/maximum").get<int>();

	UpdateView();
	UpdateProjection();

	auto light_color = conf_cam.Value<glm::vec3>("/light/position");

	light = {conf_cam.Value<glm::vec3>("/light/position"),
			 conf_cam.Value<glm::vec3>("/light/position"), light_color,
			 light_color, light_color};

	tess.Update(std::min(max_tess_fact, tess_fact));

	formater = boost::format("%1$-12s : %2%\n");

	cv::Mat default_texture = cv::imread(TEXTURE "default_texture.png");
	cv::Mat flip_texture(
		cv::Size(default_texture.size[0], default_texture.size[1]), CV_8UC3);
	cv::flip(default_texture, flip_texture, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glCreateTextures(GL_TEXTURE_2D, 1, &default_diffuse_texture);
	glTextureImage2DEXT(default_diffuse_texture, GL_TEXTURE_2D, 0,
						GL_COMPRESSED_RGB, flip_texture.cols, flip_texture.rows,
						0, GL_BGR, GL_UNSIGNED_BYTE, flip_texture.data);
	glTextureParameteri(default_diffuse_texture, GL_TEXTURE_MAG_FILTER,
						GL_LINEAR);
	glTextureParameteri(default_diffuse_texture, GL_TEXTURE_MIN_FILTER,
						GL_LINEAR);
	tv::model::default_texture = default_diffuse_texture;
}
app::~app() {
	if (default_diffuse_texture != 0u) {
		glDeleteTextures(1, &default_diffuse_texture);
	}
}

void app::Run() {
	GLFWwindow* w = win->GetWin();

	glfwSetWindowUserPointer(w, this);
	glfwSetKeyCallback(w, KeyDefaultCallback);
	glfwSetMouseButtonCallback(w, MouseButtonDfaultCallback);
	glfwSetScrollCallback(w, MouseScrollLengthCallback);
	glfwSetWindowSizeCallback(w, WindowResizeCallback);

	std::string patch_type(
		tv::model::default_patch_type ==
				OpenSubdiv::Far::PatchDescriptor::Type::REGULAR
			? "REGULAR"
			: "GREGORY_BASIS");

	query = std::make_unique<glQuery>(GL_PRIMITIVES_GENERATED);
	//draw_string_query.reset(new glQuery(GL_PRIMITIVES_GENERATED));
	glfwSetTime(0.0);
	mainloop = true;
	while (mainloop && (glfwWindowShouldClose(w) == 0)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		tv::model::draw_call = 0;

		UpdateUBO();

		query->Start();
		for (tv::model& model : models) {
			model.Draw();
		}
		frametime = (float)glfwGetTime();
		glfwSetTime(0.0);
		query->End();

		//draw_string_query->Start();
		draw_string->Set(
			5, 0,
			(boost::format("FPS          %.1f") % (1.f / frametime)).str());
		draw_string->Set(5, 18, (formater % "Tess Level" % tess_fact).str());
		draw_string->Set(5, 36, (formater % "Primitive" % query->Get()).str());
		draw_string->Set(5, 54, (formater % "Patch Type" % patch_type).str());
		draw_string->Set(
			5, 72, (formater % "Patch Level" % tv::model::default_patch).str());
		draw_string->Set(5, 90,
						 (formater % "Draw Call" % tv::model::draw_call).str());
		draw_string->Draw();
		//draw_string_query->End();

		glfwPollEvents();

		glm::dvec2 temp;
		glfwGetCursorPos(w, &temp.x, &temp.y);
		previousMousePos = temp;

		glfwSwapBuffers(w);
		glFlush();
	}
}

void app::osdErrorCallback(OpenSubdiv::Far::ErrorType err,
						   const char*                message) {
	using OpenSubdiv::Far::ErrorType;
	std::cerr << "OpenSubdiv Error Type : " << err << '\n'
			  << message << std::endl;
}
void app::osdWarningCallback(const char* message) {
	std::cerr << message << std::endl;
}
void app::glfwErrorCallback(int code, const char* message) {
	std::cerr << "GLFW ERROR Code : " << code << '\n' << message << std::endl;
}
void app::KeyDefaultCallback(
	GLFWwindow* window, int key, int scancode, int action, int mods) {
	app* a = static_cast<app*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE: a->mainloop = false; break;
		case GLFW_KEY_F:
			//glfwSetCursorPosCallback(a->win->GetWin(), nullptr);
			//glfwSetKeyCallback(a->win->GetWin(), KeyFlyModeCallback);
			//glfwSetMouseButtonCallback(a->win->GetWin(), MouseButtonFlayModeCallback);
			//a->camera.fly_mode = true;
			//std::cout << "Fly Mode" << std::endl;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			glfwSetScrollCallback(a->win->GetWin(), MouseScrollFovCallback);
			break;
		case GLFW_KEY_UP:
			a->tess_fact = glm::min(a->tess_fact + 1, a->max_tess_fact);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_DOWN:
			a->tess_fact = glm::max(a->tess_fact - 1, 0);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_SPACE: {
			auto q = a->_camera.Quaternion();
			std::cout << (a->_formater % "Pos" %
						  glm::to_string(a->_camera.Position()))
					   .str()
					  << (a->_formater % "Angle" %
						  glm::to_string(glm::degrees(glm::eulerAngles(q))))
					   .str()
					  << (a->_formater % "LookPoint" %
						  glm::to_string(q * tv::Camera::front))
					   .str()
					  << (a->_formater % "Right" %
						  glm::to_string(q * tv::Camera::right))
							 .str()
					  << (a->_formater % "Up" %
						  glm::to_string(q * tv::Camera::up))
							 .str();
		} break;
		default: break;
		}
	} else if (action == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_LEFT_SHIFT:
			glfwSetScrollCallback(a->win->GetWin(), MouseScrollLengthCallback);
			break;
		default: break;
		}
	}
}
void app::KeyFlyModeCallback(
	GLFWwindow* window, int key, int scancode, int action, int mods) {
	app* a = static_cast<app*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE: a->mainloop = false; break;
		case GLFW_KEY_F:
			glfwSetCursorPosCallback(a->win->GetWin(), nullptr);
			glfwSetKeyCallback(a->win->GetWin(), KeyDefaultCallback);
			glfwSetMouseButtonCallback(a->win->GetWin(),
									   MouseButtonDfaultCallback);
			a->camera.fly_mode = false;
			DragCameraRotate(window, 0.0, 0.0);
			std::cout << "Normal Mode" << std::endl;
			break;
		case GLFW_KEY_UP:
			a->tess_fact = glm::min(a->tess_fact + 1, a->max_tess_fact);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_DOWN:
			a->tess_fact = glm::max(a->tess_fact - 1, 0);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_W: a->_camera.FpsMove(tv::Camera::front); break;
		case GLFW_KEY_S: a->_camera.FpsMove(-tv::Camera::front); break;
		case GLFW_KEY_D: a->_camera.FpsMove(tv::Camera::right); break;
		case GLFW_KEY_A: a->_camera.FpsMove(-tv::Camera::right); break;
		case GLFW_KEY_SPACE: {
			auto q = a->_camera.Quaternion();
			std::cout << (a->_formater % "Pos" %
						  glm::to_string(a->_camera.Position()))
							 .str()
					  << (a->_formater % "Angle" %
						  glm::to_string(glm::degrees(glm::eulerAngles(q))))
					   .str()
					  << (a->_formater % "LookPoint" %
						  glm::to_string(q * tv::Camera::front))
					   .str()
					  << (a->_formater % "Right" %
						  glm::to_string(q * tv::Camera::right))
					   .str()
					  << (a->_formater % "Up" %
						  glm::to_string(q * tv::Camera::up))
							 .str();
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
	a->UpdateView();
}
	auto current = glm::vec2(x, y);
	auto move    = (current - a->_previous_mouse_pos) * a->_frametime;

	a->_camera.RotateMove(move);

	a->_previous_mouse_pos = current;
	a->_update_view();
	}

	auto current = glm::vec2(x, y);
	auto move =
		glm::vec3((current - a->_previous_mouse_pos) * a->_frametime, 0);

	a->_camera.FpsMove(move);

	a->previousMousePos.x = (float)x;
	a->previousMousePos.y = (float)y;
	a->UpdateView();
}

	auto current = glm::vec2(x, y);
	auto move    = (current - a->_previous_mouse_pos) * a->_frametime;

	a->_camera.Rotate(move);

	a->previousMousePos.x = (float)x;
	a->previousMousePos.y = (float)y;
	a->UpdateView();
}
void app::MouseButtonDfaultCallback(GLFWwindow* window,
									int         button,
									int         action,
									int         mods) {
	app* a = static_cast<app*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_MIDDLE:
			if (mods == GLFW_MOD_SHIFT) {
				glfwSetCursorPosCallback(a->win->GetWin(),
										 DragCameraTranslation);
				std::cout << "Camera Translation" << std::endl;
			} else {
				glfwSetCursorPosCallback(a->win->GetWin(), DragCameraRotate);
				std::cout << "Camera Rotate" << std::endl;
			}
			break;
		default: break;
		}
	} else if (action == GLFW_RELEASE) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_MIDDLE:
			glfwSetCursorPosCallback(a->win->GetWin(), nullptr);
			break;
		default: break;
		}
	}
}
void app::MouseButtonFlayModeCallback(GLFWwindow* window,
									  int         button,
									  int         action,
									  int         mods) {
	app* a = static_cast<app*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_MIDDLE:
			if (mods == GLFW_MOD_SHIFT) {
				glfwSetCursorPosCallback(a->win->GetWin(),
										 DragCameraTranslation);
				std::cout << "Camera Translation" << std::endl;
			} else {
				glfwSetCursorPosCallback(a->win->GetWin(), DragCameraFlyMode);
				std::cout << "Camera Rotate" << std::endl;
			}
			break;
		default: break;
		}
	} else if (action == GLFW_RELEASE) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_MIDDLE:
			glfwSetCursorPosCallback(a->win->GetWin(), nullptr);
			break;
		default: break;
		}
	}
}
void app::MouseScrollFovCallback(GLFWwindow* window, double up, double down) {
	app* a = static_cast<app*>(glfwGetWindowUserPointer(window));

	using namespace tv;
	auto fov = a->_camera.Fov() + (float)(up - down);
	a->_camera.Fov(glm::clamp(fov, Camera::MinFov(), Camera::MaxFov()));
	std::cout << "fov : " << a->_camera.Fov() << std::endl;
}
void app::MouseScrollLengthCallback(GLFWwindow* window,
									double      up,
									double      down) {
	a->_camera.Length((a->_camera.Length() * 0.1f) * (float)(down - up));

	float length = glm::length(a->camera.Pos - a->camera.LookPoint);
	a->camera.Pos += a->camera.Angle * (length * 0.1f) * (float)(down - up);
	a->UpdateView();
}
void app::WindowResizeCallback(GLFWwindow* window, int x, int y) {
	app* a = static_cast<app*>(glfwGetWindowUserPointer(window));

	glViewport(0, 0, x, y);
	a->window_size.x = x;
	a->window_size.y = y;
	a->UpdateProjection();
	a->draw_string->SetWindowSize(x, y);
}

void App::_update_view() { _transform.view = _camera.ViewMatrix(); }
void App::_update_projection() {
	_transform.projection =
		glm::perspective(glm::radians(_camera.Fov()),
						 (float)_window_size.x / (float)_window_size.y,
						 _camera.Near(), _camera.Far());
}
void app::UpdateUBO() {
	tess.Update(tess_fact);
	tv::model::default_glsl_info.GetUniformBuffer("Transform")
		->Update(&transform);
	tv::model::default_glsl_info.GetUniformBuffer("LightInfo")->Update(&light);
	tv::model::default_glsl_info.GetUniformBuffer("Tessellation")
		->Update(&tess);
	//shader_default->BindUniformBlock();
}
