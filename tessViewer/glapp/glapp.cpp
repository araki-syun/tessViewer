#include "glapp.hpp"

#include <iostream>
#include <sstream>
#include <vector>

#include "GLFW/glfw3.h"
#include "config.h"
#include "../log.h"

using tv::InfoType;
using tv::Logger;
using tv::LogLevel;
namespace glapp {

void Initialize() {
	if (glfwInit() != GL_TRUE) {
		throw tv::GraphicsError(LogLevel::Fatal, "GLFW Initialize ERROR");
	}
	Logger::Log<LogLevel::Debug>(InfoType::Graphics, "GLFW Initialize");
}

//window::window(){

//}
Window::Window(std::string_view                title,
			   int                             glversion_major,
			   int                             glversion_minor,
			   const std::vector<std::string>& required_ext)
	: inner::BaseWindow(title) {
	Logger::Log<LogLevel::Debug>(InfoType::Graphics, "Start Window Initialize");
	auto conf_win        = Config::Get("/window");
	auto conf_bit        = conf_win.Relative("/bit");
	Window::_debug_level = conf_win.Value<int>("debug_level");

	Initialize();
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, _debug_level > 0 ? 1 : 0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glversion_major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glversion_minor);
	glfwWindowHint(GLFW_SAMPLES,
				   Config::Get().Value<int>("/graphics/aa/samples"));
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
	Logger::Log<LogLevel::Debug>(InfoType::Graphics, "Create GLFW Window");

	glewExperimental = GL_TRUE;
	GLenum err       = glewInit();
	if (err != GLEW_OK) {
		throw tv::GraphicsError(tv::LogLevel::Fatal, "GLEW Initialize ERROR");
	}
	Logger::Log<LogLevel::Debug>(InfoType::Graphics, "GLEW Initialize");

	glDebugMessageCallback(_open_gl_debug_message_callback, nullptr);

	{
		std::stringstream ss;
		for (const auto& ext : required_ext) {
			if (GL_TRUE != glfwExtensionSupported(ext.c_str())) {
				ss << "UnSupported OpenGL Extension : " << ext << std::endl;
			}
		}
		if (const auto& str = ss.str(); !str.empty()) {
			throw tv::GraphicsError(tv::LogLevel::Error,
									"OpenGL Error\n" + str);
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
	Logger::Log<LogLevel::Debug>(InfoType::Graphics,
								 "Finish Window Initialize");
}
Window::Window(Window&& win) noexcept
	: inner::BaseWindow(win.GetTitle()), _win(win._win) {
	win._win = nullptr;
	Logger::Log<LogLevel::Debug>(InfoType::Graphics, "Window Move Construct");
}
Window::~Window() {
	glfwDestroyWindow(this->_win);
	glfwTerminate();
	Logger::Log<LogLevel::Debug>(InfoType::Graphics, "Window Destruct");
}
Window& Window::operator=(Window&& win) noexcept {
	if (this != &win) {
		_win     = win._win;
		win._win = nullptr;
		_title   = std::move(win._title);
	}
	Logger::Log<LogLevel::Debug>(InfoType::Graphics, "Window Move Operator");
	return *this;
}

void Window::SetBackColor(const glm::vec4& color) {
	glClearColor(color.r, color.g, color.b, color.a);
	Logger::Log<LogLevel::Debug>(InfoType::Graphics, "Set Window Back Color");
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
	int               level = 0;
	std::stringstream ss;
	std::string       severity_string;
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

	ss << "------------opengl-callback-start------------" << std::endl;

	ss << "No." << _debug_message_number << '\n';
	ss << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR: ss << "ERROR"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ss << "DEPRECATED_BEHAVIOR"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: ss << "UNDEFINED_BEHAVIOR"; break;
	case GL_DEBUG_TYPE_PORTABILITY: ss << "PORTABILITY"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: ss << "PERFORMANCE"; break;
	case GL_DEBUG_TYPE_OTHER: ss << "OTHER"; break;
	}
	ss << std::endl;

	auto errstr = reinterpret_cast<const char*>(gluErrorString(id));
	if (errstr != nullptr) {
		ss << "id: " << id << ' ' << errstr << std::endl;
	}
	ss << "severity: " << severity_string << std::endl;
	//switch (severity) {
	//case GL_DEBUG_SEVERITY_LOW:
	//	ss << "LOW";
	//	break;
	//case GL_DEBUG_SEVERITY_MEDIUM:
	//	ss << "MEDIUM";
	//	break;
	//case GL_DEBUG_SEVERITY_HIGH:
	//	ss << severity_string;
	//	break;
	//}
	//ss << std::endl;

	ss << "source: ";
	switch (source) {
	case GL_DEBUG_SOURCE_API_ARB: ss << "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB: ss << "WINDOW_SYSTEM"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: ss << "SHADER_COMPILER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB: ss << "THIRD_PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION_ARB: ss << "APPLICATION"; break;
	case GL_DEBUG_SOURCE_OTHER_ARB: ss << "OTHER"; break;
	}
	ss << std::endl;

	ss << "message: " << message << std::endl;
	ss << "------------opengl-callback-end------------" << std::endl;
	tv::LogLevel lv;
	switch (severity) {
	case GL_DEBUG_SEVERITY_NOTIFICATION: lv = tv::LogLevel::Notice; break;
	case GL_DEBUG_SEVERITY_LOW: lv = tv::LogLevel::Warning; break;
	case GL_DEBUG_SEVERITY_MEDIUM: lv = tv::LogLevel::Error; break;
	case GL_DEBUG_SEVERITY_HIGH: lv = tv::LogLevel::Error; break;
	}
	tv::Logger::Log(lv, tv::InfoType::Graphics, ss.str());
	++_debug_message_number;
}

} // namespace glapp