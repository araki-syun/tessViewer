#pragma once

#include <cstdint>
#include <memory>

#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <string_view>
#include <vector>

#include "base_window.h"
#include "config.h"
#include "shader_program.h"

namespace glapp {

void Initialize();

class Window : public inner::BaseWindow {
public:
	Window() = delete;
	explicit Window(std::string_view                title,
					int                             glversion_major,
					int                             glversion_minor,
					const std::vector<std::string>& required_ext);
	Window(const Window&) = delete;
	Window(Window&& win) noexcept;
	~Window() override;
	Window& operator=(const Window&) = delete;
	Window& operator                 =(Window&& win) noexcept;

	void        SetBackColor(const glm::vec4& color);
	void        SetWindowSize(int w, int h);
	void        SetWindowSize(glm::ivec2 size);
	glm::ivec2  GetWindowSize() const;
	void        GetWindowSize(int* x, int* y) const override;
	void        GetWindowPosition(int* x, int* y) const override;
	GLuint      GetFrameBuffer() const;
	GLFWwindow* GetWin();

public:
	//std::unique_ptr<glShaderProgram> program;

private:
	GLFWwindow* _win;

	static int          _debug_level;
	static unsigned int _debug_message_number;
	static void GLAPIENTRY
	_open_gl_debug_message_callback(GLenum        source,
									GLenum        type,
									GLuint        id,
									GLenum        severity,
									GLsizei       length,
									const GLchar* message,
									const void*   user_param);
};

} // namespace glapp
