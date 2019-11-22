#include "glapp.hpp"

#include <iostream>

#include "GLFW/glfw3.h"
#include "config.h"

namespace glapp {

void Initialize() {
	if (glfwInit() != GL_TRUE) {
		throw std::runtime_error("GLFW Initialize ERROR");
	}
}

//window::window(){

//}
Window::Window(std::string_view                title,
			   int                             glversion_major,
			   int                             glversion_minor,
			   const std::vector<std::string>& required_ext)
	: inner::BaseWindow(title) {
	auto conf_win        = Config::Get("/window");
	auto conf_bit        = conf_win.Relative("/bit");
	Window::_debug_level = conf_win.Value<int>("debug_level");

	Initialize();
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, _debug_level > 0 ? 1 : 0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glversion_major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glversion_minor);
	glfwWindowHint(GLFW_SAMPLES, conf_win.Value<int>("/graphics/aa/samples"));
	glfwWindowHint(GLFW_RED_BITS, conf_bit.Value<int>("red"));
	glfwWindowHint(GLFW_GREEN_BITS, conf_bit.Value<int>("green"));
	glfwWindowHint(GLFW_BLUE_BITS, conf_bit.Value<int>("blue"));
	glfwWindowHint(GLFW_ALPHA_BITS, conf_bit.Value<int>("alpha"));
	glfwWindowHint(GLFW_DEPTH_BITS, conf_bit.Value<int>("depth"));
	glfwWindowHint(GLFW_STENCIL_BITS, conf_bit.Value<int>("stencil"));

	this->_win = glfwCreateWindow(
		conf_win.Value<int>("resolution/width"),
		conf_win.Value<int>("resolution/height"), std::string(title).c_str(),
		conf_win.Value<bool>("fullscreen") ? glfwGetPrimaryMonitor() : nullptr,
		nullptr);
	glfwMakeContextCurrent(this->_win);

	glewExperimental = GL_TRUE;
	GLenum err       = glewInit();
	if (err != GLEW_OK) {
		throw std::runtime_error("GLEW Initialize ERROR");
	}

	glDebugMessageCallback(_open_gl_debug_message_callback, nullptr);

	for (const auto& ext : required_ext) {
		if (GL_TRUE != glfwExtensionSupported(ext.c_str())) {
			throw std::runtime_error("OpenGL Unsupported : " + ext);
		}
	}

	//program.reset(new glShaderProgram());

	glfwSwapInterval(conf_win.Value<bool>("vsync") ? 1 : 0);

	glViewport(0, 0, conf_win.Value<int>("resolution/width"),
			   conf_win.Value<int>("resolution/height"));
	SetBackColor(conf_win.Value<glm::vec4>("background_color"));
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
}
Window::Window(Window&& win) noexcept
	: inner::BaseWindow(win.GetTitle()), _win(win._win) {
	win._win = nullptr;
}
Window::~Window() {
	glfwDestroyWindow(this->_win);
	glfwTerminate();
}
Window& Window::operator=(Window&& win) noexcept {
	if (this != &win) {
		_win     = win._win;
		win._win = nullptr;
		_title   = std::move(win._title);
	}
	return *this;
}

void Window::SetBackColor(const glm::vec4& color) {
	glClearColor(color.r, color.g, color.b, color.a);
}

glm::ivec2 Window::GetWindowSize() const {
	glm::ivec2 size(0);
	GetWindowSize(&size.x, &size.y);
	return size;
}
void Window::GetWindowSize(int* x, int* y) const {
	glfwGetWindowSize(_win, x, y);
}
void glapp::Window::GetWindowPosition(int* x, int* y) const {
	glfwGetWindowPos(_win, x, y);
}
GLuint      glapp::Window::GetFrameBuffer() const { return GLuint(); }
GLFWwindow* Window::GetWin() { return _win; }

int          Window::_debug_level          = 0;
unsigned int Window::_debug_message_number = 0;
void GLAPIENTRY
Window::_open_gl_debug_message_callback(GLenum        source,
										GLenum        type,
										GLuint        id,
										GLenum        severity,
										GLsizei       length,
										const GLchar* message,
										const void*   user_param) {
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