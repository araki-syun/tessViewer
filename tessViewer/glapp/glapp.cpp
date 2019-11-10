#include "glapp.hpp"

#include <iostream>

#include "glapp_define.h"
#include "config.h"

namespace glapp {

void Initialize() {
	if (glfwInit() != GL_TRUE) {
		throw std::runtime_error("GLFW Initialize ERROR");
	}
}

//window::window(){

//}
window::window(const char* title,
			   int         width,
			   int         height,
			   int         glversion_major,
			   int         glversion_minor,
			   int         samples,
			   int         vsync,
			   init_flag   flag)
	: inner::base_window(title, width, height, flag) {
	auto conf_win        = Config::Get("/window");
	auto conf_bit        = conf_win.Relative("/bit");
	window::_debug_level = conf_win.Value<int>("debug_level");

	Initialize();
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, _debug_level > 0 ? 1 : 0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glversion_major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glversion_minor);
	glfwWindowHint(GLFW_SAMPLES, samples);
	glfwWindowHint(GLFW_RED_BITS, conf_bit.Value<int>("red"));
	glfwWindowHint(GLFW_GREEN_BITS, conf_bit.Value<int>("green"));
	glfwWindowHint(GLFW_BLUE_BITS, conf_bit.Value<int>("blue"));
	glfwWindowHint(GLFW_ALPHA_BITS, conf_bit.Value<int>("alpha"));
	glfwWindowHint(GLFW_DEPTH_BITS, conf_bit.Value<int>("depth"));
	glfwWindowHint(GLFW_STENCIL_BITS, conf_bit.Value<int>("stencil"));

	this->_win =
		glfwCreateWindow(width, height, title,
						 (static_cast<std::uint8_t>(flag) &
						  static_cast<std::uint8_t>(
							  inner::base_window::init_flag::FULLSCREEN)) != 0
							 ? glfwGetPrimaryMonitor()
							 : nullptr,
						 nullptr);
	glfwMakeContextCurrent(this->_win);

	glewExperimental = GL_TRUE;
	GLenum err       = glewInit();
	if (err != GLEW_OK) {
		throw std::runtime_error("GLEW Initialize ERROR");
	}

	glDebugMessageCallback(openGLDebugMessageCallback, nullptr);

	//program.reset(new glShaderProgram());

	glfwSwapInterval(conf_win.Value<bool>("vsync") ? 1 : 0);

	glViewport(0, 0, conf_win.Value<int>("resolution/width"),
			   conf_win.Value<int>("resolution/height"));
	SetBackColor(conf_win.Value<glm::vec4>("background_color"));
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
}
window::~window() {
	glfwDestroyWindow(this->_win);
	glfwTerminate();
}

void window::SetBackColor(const glm::vec4& color) {
	glClearColor(color.r, color.g, color.b, color.a);
}

glm::ivec2 window::GetWindowSize() const {
	glm::ivec2 size(0);
	GetWindowSize(&size.x, &size.y);
	return size;
}
void window::GetWindowSize(int* x, int* y) const {
	glfwGetWindowSize(_win, x, y);
}
void glapp::window::GetWindowPosition(int* x, int* y) const {
	glfwGetWindowPos(_win, x, y);
}
GLuint      glapp::window::GetFrameBuffer() const { return GLuint(); }
GLFWwindow* window::GetWin() { return _win; }

int             window::_debug_level          = 0;
unsigned int    window::_debug_message_number = 0;
void GLAPIENTRY window::openGLDebugMessageCallback(GLenum        source,
												   GLenum        type,
												   GLuint        id,
												   GLenum        severity,
												   GLsizei       length,
												   const GLchar* message,
												   const void*   userParam) {
	int         level = 0;
	std::string severity_string;
	switch (severity) {
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		severity_string = "NOTIFICATION";
		level           = 0;
		break;
	case GL_DEBUG_SEVERITY_LOW:
		severity_string = "LOW";
		level           = 1;
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		severity_string = "MEDIUM";
		level           = 2;
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		severity_string = "HIGH";
		level           = 3;
		break;
	}
	if (level < _debug_level) {
		return;
	}

	std::cout << "---------------------opengl-callback-start------------"
			  << std::endl;

	std::cout << "No." << _debug_message_number << '\n';
	std::cout << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR: std::cout << "ERROR"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		std::cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		std::cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY: std::cout << "PORTABILITY"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "PERFORMANCE"; break;
	case GL_DEBUG_TYPE_OTHER: std::cout << "OTHER"; break;
	}
	std::cout << std::endl;

	auto errstr = reinterpret_cast<const char*>(gluErrorString(id));
	if (errstr != nullptr) {
		std::cout << "id: " << id << ' ' << errstr << std::endl;
	}
	std::cout << "severity: " << severity_string << std::endl;
	//switch (severity) {
	//case GL_DEBUG_SEVERITY_LOW:
	//	std::cout << "LOW";
	//	break;
	//case GL_DEBUG_SEVERITY_MEDIUM:
	//	std::cout << "MEDIUM";
	//	break;
	//case GL_DEBUG_SEVERITY_HIGH:
	//	std::cout << severity_string;
	//	break;
	//}
	//std::cout << std::endl;

	std::cout << "source: ";
	switch (source) {
	case GL_DEBUG_SOURCE_API_ARB: std::cout << "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB: std::cout << "WINDOW_SYSTEM"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
		std::cout << "SHADER_COMPILER";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB: std::cout << "THIRD_PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION_ARB: std::cout << "APPLICATION"; break;
	case GL_DEBUG_SOURCE_OTHER_ARB: std::cout << "OTHER"; break;
	}
	std::cout << std::endl;

	std::cout << "message: " << message << std::endl;
	std::cout << "---------------------opengl-callback-end--------------"
			  << std::endl;
	++_debug_message_number;
}

} // namespace glapp