#pragma once

#include <cstdint>
#include <memory>

#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>

#include "base_window.h"
#include "config.h"
#include "glslprogram.h"

namespace glapp {

void Initialize();

class window : public inner::base_window {
public:
	explicit window(const char* title,
					int         width           = 1280,
					int         height          = 720,
					int         glversion_major = 4,
					int         glversion_minor = 3,
					int         samples         = 2,
					int         vsync           = 1,
					init_flag   flag            = init_flag::DEFAULT);
	virtual ~window();

	void             SetBackColor(const glm::vec4& color);
	const glm::ivec2 GetWindowSize() const;
	void             GetWindowSize(int* w, int* h) const override;
	void             GetWindowPosition(int* x, int* y) const override;
	GLuint           GetFrameBuffer() const;
	GLFWwindow*      GetWin();

public:
	//std::unique_ptr<glslprogram> program;

private:
	GLFWwindow* _win;
};

} // namespace glapp
