#pragma once

#include <vector>
#include <memory>
#include <cstdint>

#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\ext.hpp>

#include <opensubdiv\far\error.h>

#include "glapp\glapp.hpp"

#include "ShaderManager.h"
#include "model.h"
#include "glslStringDraw.h"
#include "glQuery.h"
#include "material.h"
#include "camera.h"

class App {
public:
	App();
	App(const App&) = delete;
	App(App&&)      = delete;
	// app(boost::program_options::variables_map& vm);
	~App();
	App& operator=(const App&) = delete;
	App& operator=(App&&) = delete;

	void Run();

	static void OsdErrorCallback(OpenSubdiv::Far::ErrorType err,
								 const char*                message);
	static void OsdWarningCallback(const char* message);
	static void GlfwErrorCallback(int code, const char* message);

	static void KeyDefaultCallback(
		GLFWwindow* window, int key, int scancode, int action, int mods);
	static void KeyFlyModeCallback(
		GLFWwindow* window, int key, int scancode, int action, int mods);
	static void DragCameraRotate(GLFWwindow* window, double x, double y);
	static void DragCameraTranslation(GLFWwindow* window, double x, double y);
	static void DragCameraFlyMode(GLFWwindow* window, double x, double y);
	static void MouseButtonDfaultCallback(GLFWwindow* window,
										  int         button,
										  int         action,
										  int         mods);
	static void MouseButtonFlayModeCallback(GLFWwindow* window,
											int         button,
											int         action,
											int         mods);
	static void
	MouseScrollFovCallback(GLFWwindow* window, double up, double down);
	static void
	MouseScrollLengthCallback(GLFWwindow* window, double up, double down);

	static void WindowResizeCallback(GLFWwindow* window, int x, int y);

private:
	void _update_view();
	void _update_projection();
	void _update_ubo();

	std::unique_ptr<glapp::Window> _win;
	std::vector<tv::Model>         _models;
	std::shared_ptr<tv::Material>  _material;
	//std::shared_ptr<glslProgram> shader_default;
	tv::ShaderManager _shader_manage;

	tv::GlslStringDraw* _draw_string{};
	GLuint              _default_diffuse_texture = 0;

	bool       _mainloop{};
	float      _frametime{};
	tv::Camera _camera;
	bool       _fly_mode = false;
	int        _tess_fact{};
	int        _max_tess_fact{};
	glm::vec2  _mouse_move{};
	glm::vec2  _previous_mouse_pos{};

	std::unique_ptr<GlQuery> _query;
	//std::unique_ptr<glQuery> draw_string_query;

	struct Transform {
		glm::mat4 view;
		glm::mat4 projection;
	} _transform{};
	struct LightInfo {
		glm::vec3 pos;
		glm::vec3 vector;
		glm::vec3 ia;
		glm::vec3 id;
		glm::vec3 is;
	} _light{};
	struct Tessellation {
		float TessLevel;
		void  Update(int t) { TessLevel = static_cast<float>(1 << t); }
	} _tess{};
};
