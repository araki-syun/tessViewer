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
#include "glShaderProgram.h"

namespace glapp {

void Initialize();

class window : public inner::base_window {
public:
	explicit window(std::string_view                title,
					int                             glversion_major,
					int                             glversion_minor,
					const std::vector<std::string>& required_ext);
	window(const window&) = delete;
	~window() override;

	void        SetBackColor(const glm::vec4& color);
	glm::ivec2  GetWindowSize() const;
	void        GetWindowSize(int* x, int* y) const override;
	void        GetWindowPosition(int* x, int* y) const override;
	GLuint      GetFrameBuffer() const;
	GLFWwindow* GetWin();

public:
	//std::unique_ptr<glShaderProgram> program;

private:
	GLFWwindow* _win;

	static int             _debug_level;
	static unsigned int    _debug_message_number;
	static void GLAPIENTRY openGLDebugMessageCallback(GLenum        source,
													  GLenum        type,
													  GLuint        id,
													  GLenum        severity,
													  GLsizei       length,
													  const GLchar* message,
													  const void*   userParam);
};

} // namespace glapp
