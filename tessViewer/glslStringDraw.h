#pragma once

#include <vector>

#include <boost\noncopyable.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL\glew.h>
#include <glm\glm.hpp>
#include <glm\ext.hpp>

#include "location_define.h"
#include "glslProgram.h"

#define FONT_VERTEX_SHADER SHADER R"(default_font.vert)"
#define FONT_FRAGMENT_SHADER SHADER R"(default_font.frag)"

namespace tv {
class GlslStringDraw : public boost::noncopyable {
public:
	~GlslStringDraw();

	static GlslStringDraw* GetInstance();

	void Initialize(int                fontsize = 12,
					const std::string& fontname = FONT "ipaexg.ttf");
	void Set(int x, int y, const std::string& str);
	void Draw();
	void SetWindowSize(int w, int h);

private:
	GlslStringDraw();
	static GlslStringDraw stringDraw;

	GLuint     _texture{0};
	GLuint     _sampler{0};
	FT_Library _ftlib{};
	FT_Face    _ftface{};
	int        _font_size{};
	int        _fonttex_size{};
	glm::ivec2 _window_size{};
	struct CharctorUV {
		//glm::vec2 uv[4];
		glm::vec2 xy, wh;
	};
	std::vector<CharctorUV>   _list;
	std::vector<glm::vec2>    _buffer;
	std::vector<unsigned int> _element;
	int                       _element_offset{};
	GLuint                    _vao{0}, _vbo{0}, _ebo{0};
	GlslProgram               _program;
};
} // namespace tv
