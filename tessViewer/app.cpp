#include "app.h"

#include <fstream>
#include <cstdint>

#include <opencv2\core.hpp>
#include <opencv2\imgcodecs.hpp>
#include <opencv2\highgui.hpp>

#include "picojson.h"

#include "glapp\config.h"
#include "glapp\glapp_define.h"

#include "version.h"

app::app(boost::program_options::variables_map& vm) {
	current_model = vm["object"].as<std::string>();

	setting.window_full_screen  = vm.count(GLAPP_CONFIG_FULLSCREEN) > 0;
	setting.window_resolution_x = vm[GLAPP_CONFIG_RESOLUTION_X].as<int>();
	setting.window_resolution_y = vm[GLAPP_CONFIG_RESOLUTION_Y].as<int>();
	setting.window_fov          = vm[GLAPP_CONFIG_FOV].as<float>();
	setting.window_vsync        = vm[GLAPP_CONFIG_VSYNC].as<bool>();
	setting.graphics_osd_patch_level_default =
		vm[GLAPP_CONFIG_PATCH_LEVEL_DEFAULT].as<int>();
	setting.graphics_osd_patch_level_max =
		vm[GLAPP_CONFIG_PATCH_LEVEL_MAX].as<int>();
	setting.graphics_osd_tess_level_default =
		vm[GLAPP_CONFIG_TESS_LEVEL_DEFAULT].as<int>();
	setting.graphics_osd_tess_level_max =
		vm[GLAPP_CONFIG_TESS_LEVEL_MAX].as<int>();
	setting.ui_user_interface = vm[GLAPP_CONFIG_USER_INTERFACE].as<bool>();
	setting.ui_font_file      = std::string(FONT R"(ipaexg.ttf)");
	setting.ui_font_size      = vm[GLAPP_CONFIG_FONT_SIZE].as<int>();
	setting.ui_font_color     = vm[GLAPP_CONFIG_FONT_COLOR].as<std::uint8_t>();
	tv::model::default_patch_type =
		vm.count(GLAPP_CONFIG_PATCH_TYPE_GREGORY) > 0
			? OpenSubdiv::Far::PatchDescriptor::Type::GREGORY_BASIS
			: OpenSubdiv::Far::PatchDescriptor::Type::REGULAR;

	if (current_model.empty())
		throw std::runtime_error("Model Import -o [file_path]\n");

	OpenSubdiv::Far::SetErrorCallback(osdErrorCallback);
	OpenSubdiv::Far::SetWarningCallback(osdWarningCallback);

	glfwSetErrorCallback(glfwErrorCallback);

	win.reset(new glapp::window(
		"tessViewer " TV_VERSION, setting.window_resolution_x,
		setting.window_resolution_y, 4, 5, 4, (int)setting.window_vsync,
		setting.window_full_screen ? glapp::window::init_flag::FULLSCREEN
								   : glapp::window::init_flag::DEFAULT));
	win->SetBackColor(glm::vec4(glm::vec3(0.5f), 1));

	if (GL_TRUE != glfwExtensionSupported("GL_ARB_direct_state_access"))
		throw std::runtime_error(
			"OpenGL Unsupported : GL_ARB_direct_state_access");
	if (GL_TRUE != glfwExtensionSupported("GL_ARB_tessellation_shader"))
		throw std::runtime_error(
			"OpenGL Unsupported : GL_ARB_tessellation_shader");

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

	tv::model::max_patch     = setting.graphics_osd_patch_level_max;
	tv::model::default_patch = setting.graphics_osd_patch_level_default;

	draw_string = tv::glslStringDraw::getInstance();
	draw_string->Initialize(setting.ui_font_size, setting.ui_font_file);
	draw_string->SetWindowSize(setting.window_resolution_x,
							   setting.window_resolution_y);

	std::ifstream sdmj(current_model, std::ios::in);
	if (!sdmj)
		throw std::runtime_error("File Open Error\n");

	picojson::value   v;
	std::string       err     = picojson::parse(v, sdmj);
	picojson::object& o       = v.get<picojson::object>();
	picojson::array&  objects = o["objects"].get<picojson::array>();
	material.reset(new tv::material(o["materials"].get<picojson::array>()));
	models.reserve(objects.size());
	for (picojson::value& o : objects) {
		models.emplace_back(o.get<picojson::object>(),
							/*shader_default,*/ material);
	}

	window_size =
		glm::ivec2(setting.window_resolution_x, setting.window_resolution_y);

	camera.Pos       = glm::vec3(0.0f, 0.0f, 3.0f);
	camera.LookPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.Angle     = glm::normalize(camera.LookPoint - camera.Pos);
	camera.Fov       = setting.window_fov;
	camera.Right     = glm::cross(camera.Angle, glm::vec3(0, 1, 0));
	camera.Up        = glm::cross(camera.Right, camera.Angle);

	tess_fact     = setting.graphics_osd_tess_level_default;
	max_tess_fact = setting.graphics_osd_tess_level_max;

	UpdateView();
	UpdateProjection();
	light = {glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f),
			 glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
			 glm::vec3(0.5f, 0.5f, 0.5f)};
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
app::~app(void) {
	if (default_diffuse_texture)
		glDeleteTextures(1, &default_diffuse_texture);
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

	query.reset(new glQuery(GL_PRIMITIVES_GENERATED));
	//draw_string_query.reset(new glQuery(GL_PRIMITIVES_GENERATED));
	glfwSetTime(0.0);
	mainloop = true;
	while (mainloop && !glfwWindowShouldClose(w)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		tv::model::draw_call = 0;

		UpdateUBO();

		query->Start();
		for (tv::model& model : models)
			model.Draw();
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
	app* a = (app*)glfwGetWindowUserPointer(window);

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
			a->tess_fact = glm::min(a->tess_fact + 1,
									a->setting.graphics_osd_tess_level_max);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_DOWN:
			a->tess_fact = glm::max(a->tess_fact - 1, 0);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_SPACE:
			std::cout
				//<< (a->format % "FPS" % (1.f / a->frametime)).str()
				//<< (a->format % "Primitive" % (int)a->query->Get()).str()
				//<< (a->format % "Text Prim" % (int)a->draw_string_query->Get()).str()
				<< (a->formater % "Pos" % glm::to_string(a->camera.Pos)).str()
				<< (a->formater % "Angle" % glm::to_string(a->camera.Angle))
					   .str()
				<< (a->formater % "LookPoint" %
					glm::to_string(a->camera.LookPoint))
					   .str()
				<< (a->formater % "Right" % glm::to_string(a->camera.Right))
					   .str()
				<< (a->formater % "Up" % glm::to_string(a->camera.Up)).str();
			break;
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
	app* a = (app*)glfwGetWindowUserPointer(window);

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
			a->tess_fact = glm::min(a->tess_fact + 1,
									a->setting.graphics_osd_tess_level_max);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_DOWN:
			a->tess_fact = glm::max(a->tess_fact - 1, 0);
			//std::cout << a->format % "TessLevel" % a->tess_fact;
			break;
		case GLFW_KEY_W: a->camera.Move.y = 1.f; break;
		case GLFW_KEY_S: a->camera.Move.y = -1.f; break;
		case GLFW_KEY_D: a->camera.Move.x = 1.f; break;
		case GLFW_KEY_A: a->camera.Move.x = -1.f; break;
		case GLFW_KEY_SPACE:
			std::cout
				//<< (a->format % "FPS" % (1.f / a->frametime)).str()
				//<< (a->format % "Primitive" % (int)a->query->Get()).str()
				//<< (a->format % "Text Prim" % (int)a->draw_string_query->Get()).str()
				<< (a->formater % "Pos" % glm::to_string(a->camera.Pos)).str()
				<< (a->formater % "Angle" % glm::to_string(a->camera.Angle))
					   .str()
				<< (a->formater % "LookPoint" %
					glm::to_string(a->camera.LookPoint))
					   .str()
				<< (a->formater % "Right" % glm::to_string(a->camera.Right))
					   .str()
				<< (a->formater % "Up" % glm::to_string(a->camera.Up)).str();
			break;
		default: break;
		}
	}
	if (action == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_W: a->camera.Move.y = 0.f; break;
		case GLFW_KEY_S: a->camera.Move.y = 0.f; break;
		case GLFW_KEY_D: a->camera.Move.x = 0.f; break;
		case GLFW_KEY_A: a->camera.Move.x = 0.f; break;
		default: break;
		}
	}
	a->UpdateView();
}
void app::DragCameraRotate(GLFWwindow* window, double x, double y) {
	app* a = (app*)glfwGetWindowUserPointer(window);

	glm::vec2 move((float)x - a->previousMousePos.x,
				   (float)y - a->previousMousePos.y);
	move *= a->frametime;
	if (move.x != 0.f) {
		glm::vec3 o(a->camera.Pos - a->camera.LookPoint);
		a->camera.Pos   = glm::rotateY(o, -move.x) + a->camera.LookPoint;
		a->camera.Angle = glm::normalize(a->camera.LookPoint - a->camera.Pos);
		a->camera.Right = glm::cross(a->camera.Angle, glm::vec3(0.f, 1.f, 0.f));
		a->camera.Up    = glm::cross(a->camera.Right, a->camera.Angle);
	}
	if (move.y != 0.f) {
		glm::vec3 o(a->camera.Pos - a->camera.LookPoint);
		a->camera.Pos = glm::rotate(a->camera.Pos - a->camera.LookPoint, move.y,
									-a->camera.Right) +
						a->camera.LookPoint;
		a->camera.Angle = glm::normalize(a->camera.LookPoint - a->camera.Pos);
		a->camera.Right = glm::cross(a->camera.Angle, glm::vec3(0.f, 1.f, 0.f));
		a->camera.Up    = glm::cross(a->camera.Right, a->camera.Angle);
	}

	a->previousMousePos.x = (float)x;
	a->previousMousePos.y = (float)y;
	a->UpdateView();
}
void app::DragCameraTranslation(GLFWwindow* window, double x, double y) {
	app* a = (app*)glfwGetWindowUserPointer(window);

	glm::vec2 move((float)x - a->previousMousePos.x,
				   (float)y - a->previousMousePos.y);
	move *=
		a->frametime * 0.1f * glm::length(a->camera.Pos - a->camera.LookPoint);
	glm::vec3 trans(a->camera.Right * -move.x + a->camera.Up * move.y);
	a->camera.Pos += trans;
	a->camera.LookPoint += trans;

	a->previousMousePos.x = (float)x;
	a->previousMousePos.y = (float)y;
	a->UpdateView();
}
void app::DragCameraFlyMode(GLFWwindow* window, double x, double y) {
	app* a = (app*)glfwGetWindowUserPointer(window);

	glm::vec2 move((float)x - a->previousMousePos.x,
				   (float)y - a->previousMousePos.y);
	move *= a->frametime;
	if (move.x != 0.f) {
		a->camera.Angle = glm::rotateY(a->camera.Angle, -move.x);
		a->camera.Right = glm::cross(a->camera.Angle, glm::vec3(0, 1, 0));
		a->camera.Up    = glm::cross(a->camera.Right, a->camera.Angle);
		//a->camera.LookPoint = glm::rotateY(a->camera.LookPoint, -move.x);
	}
	if (move.y != 0.f) {
		a->camera.Angle =
			glm::rotate(a->camera.Angle, -move.y, a->camera.Right);
		a->camera.Right = glm::cross(a->camera.Angle, glm::vec3(0, 1, 0));
		a->camera.Up    = glm::cross(a->camera.Right, a->camera.Angle);
		//a->camera.LookPoint = glm::rotate(a->camera.LookPoint, -move.y, a->camera.Right);
	}

	a->previousMousePos.x = (float)x;
	a->previousMousePos.y = (float)y;
	a->UpdateView();
}
void app::MouseButtonDfaultCallback(GLFWwindow* window,
									int         button,
									int         action,
									int         mods) {
	app* a = (app*)glfwGetWindowUserPointer(window);

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
	app* a = (app*)glfwGetWindowUserPointer(window);

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
	app* a = (app*)glfwGetWindowUserPointer(window);

	a->camera.Fov += (float)(up - down);
	a->camera.Fov = glm::clamp(a->camera.Fov, 5.f, 120.f);
	a->UpdateProjection();
	std::cout << "fov : " << a->camera.Fov << std::endl;
}
void app::MouseScrollLengthCallback(GLFWwindow* window,
									double      up,
									double      down) {
	app* a = (app*)glfwGetWindowUserPointer(window);

	float length = glm::length(a->camera.Pos - a->camera.LookPoint);
	a->camera.Pos += a->camera.Angle * (length * 0.1f) * (float)(down - up);
	a->UpdateView();
}
void app::WindowResizeCallback(GLFWwindow* window, int x, int y) {
	app* a = (app*)glfwGetWindowUserPointer(window);

	glViewport(0, 0, x, y);
	a->window_size.x = x;
	a->window_size.y = y;
	a->UpdateProjection();
	a->draw_string->SetWindowSize(x, y);
}

void app::UpdateView() {
	if (camera.fly_mode) {
		if (glm::length(camera.Move) > 0.1f) {
			glm::vec3 m = glm::normalize(camera.Angle * camera.Move.y +
										 camera.Right * camera.Move.x) *
						  frametime;
			camera.Pos += m;
			camera.LookPoint += m;
		}
	}
	transform.view = glm::lookAt(camera.Pos, camera.Pos + camera.Angle,
								 camera.Up); // glm::vec3(0, 1, 0));
}
void app::UpdateProjection() {
	transform.projection = glm::perspective(
		glm::radians(camera.Fov), (float)window_size.x / (float)window_size.y,
		0.01f, 100.0f);
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
