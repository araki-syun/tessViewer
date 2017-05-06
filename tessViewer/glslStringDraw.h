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
	class glslStringDraw : public boost::noncopyable
	{
	public:
		~glslStringDraw();

		static glslStringDraw* getInstance();

		void Initialize(int fontsize = 12, const std::string& fontname = FONT "ipaexg.ttf");
		void Set(int x, int y, const std::string& str);
		void Draw();
		void SetWindowSize(int w, int h);

	private:
		glslStringDraw();
		static glslStringDraw stringDraw;

		GLuint texture;
		GLuint sampler;
		FT_Library ftlib;
		FT_Face ftface;
		int font_size;
		int fonttex_size;
		glm::ivec2 window_size;
		struct CharctorUV {
			//glm::vec2 uv[4];
			glm::vec2 xy, wh;
		};
		std::vector<CharctorUV> list;
		std::vector<glm::vec2> buffer;
		std::vector<unsigned int> element;
		int element_offset;
		GLuint vao, vbo, ebo;
		glslProgram program;
	};
}

