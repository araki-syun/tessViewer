#pragma once

#include <vector>
#include <memory>
#include <boost\format.hpp>
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

class app {
public:
	app();
	app(const app&) = delete;
	app(app&&)      = delete;
	// app(boost::program_options::variables_map& vm);
	~app();
	app& operator=(const app&) = delete;
	app& operator=(app&&) = delete;

	void Run();

	static void osdErrorCallback(OpenSubdiv::Far::ErrorType err,
								 const char*                message);
	static void osdWarningCallback(const char* message);
	static void glfwErrorCallback(int code, const char* message);

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
	void UpdateView();
	void UpdateProjection();
	void UpdateUBO();

	std::unique_ptr<glapp::window> win;
	std::vector<tv::model>         models;
	std::shared_ptr<tv::material>  material;
	//std::shared_ptr<glslProgram> shader_default;
	tv::ShaderManager shader_manage;

	tv::glslStringDraw* draw_string;
	GLuint              default_diffuse_texture = 0;

	bool       mainloop{};
	glm::ivec2 window_size{};
	float      frametime{};
	struct Camera {
		glm::vec3 Pos;
		glm::vec3 Angle;
		glm::vec3 LookPoint;
		glm::vec3 Right;
		glm::vec3 Up;
		float     Fov;
		float     maxFov;
		float     minFov;
		float     Near;
		float     Far;
		glm::vec2 Move;
		bool      fly_mode;
	} camera{};
	int       tess_fact;
	int       max_tess_fact;
	glm::vec2 mouseMove{};
	glm::vec2 previousMousePos{};

	std::unique_ptr<glQuery> query;
	//std::unique_ptr<glQuery> draw_string_query;

	struct Transform {
		glm::mat4 view;
		glm::mat4 projection;
	} transform{};
	struct LightInfo {
		glm::vec3 pos;
		glm::vec3 vector;
		glm::vec3 ia;
		glm::vec3 id;
		glm::vec3 is;
	} light{};
	struct Tessellation {
		float TessLevel;
		void  Update(int t) { TessLevel = static_cast<float>(1 << t); }
	} tess{};

	boost::format formater;
};
