#include "glslStringDraw.h"

#include <boost\format.hpp>

#include <opencv2\core.hpp>
#include <opencv2\imgcodecs.hpp>
#include <opencv2\highgui.hpp>

#define NUM_CBLOCK 16

namespace tv {
glslStringDraw::glslStringDraw()
	: texture(0), sampler(0), vao(0), vbo(0), ebo(0) {}

glslStringDraw::~glslStringDraw() {
	FT_Done_Face(ftface);
	FT_Done_FreeType(ftlib);
	if (texture)
		glDeleteTextures(1, &texture);
	if (sampler)
		glDeleteSamplers(1, &sampler);
	if (vao)
		glDeleteVertexArrays(1, &vao);
}

glslStringDraw* glslStringDraw::getInstance() { return &stringDraw; }

void glslStringDraw::Initialize(int fontsize, const std::string& fontname) {
	font_size = fontsize;
	if (FT_Init_FreeType(&ftlib))
		throw std::exception("ERROR : freetype Initialize");
	if (FT_New_Face(ftlib, fontname.c_str(), 0, &ftface))
		throw std::exception("ERROR : font file Load\n");
	if (FT_Set_Char_Size(ftface, 0, font_size * 64, 80, 80))
		throw std::exception("ERROR : set font char size\n");
	//if(FT_Set_Pixel_Sizes(this->_face, w, h))
	//	throw std::exception("ERROR : font set pixel\n");

	fonttex_size          = font_size * NUM_CBLOCK;
	unsigned int tex_size = 1;
	while (tex_size < (unsigned int)fonttex_size)
		tex_size <<= 1;
	GLuint temp_texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &temp_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureImage2DEXT(temp_texture, GL_TEXTURE_2D, 0, GL_RED, tex_size,
						tex_size, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	list.resize('~' - ' ');
	for (int i = ' '; i < '~'; ++i) {
		FT_UInt index = FT_Get_Char_Index(ftface, i);
		if (FT_Load_Char(ftface, i, FT_LOAD_DEFAULT))
			throw std::runtime_error(
				(boost::format("Load Char : %1%") % (char)i).str());
		if (FT_Render_Glyph(ftface->glyph,
							FT_Render_Mode::FT_RENDER_MODE_NORMAL))
			throw std::runtime_error("Render Gliph");
		FT_Bitmap& bm = ftface->glyph->bitmap;

		glm::ivec2 pos((i & 0x0F) * (fonttex_size / font_size),
					   ((i & 0xF0) >> 4) * (fonttex_size / font_size));

		glTextureSubImage2D(
			temp_texture, 0, pos.x + ftface->glyph->metrics.horiBearingX / 64,
			pos.y + font_size -
				ftface->glyph->metrics.horiBearingY / 64, //- bm->top_margin,
			bm.width, bm.rows, GL_RED, GL_UNSIGNED_BYTE, bm.buffer);
		glm::vec2 fpos(pos.x, tex_size - pos.y);
		//list[i - ' '].uv[0] = fpos / (float)fonttex_size;
		//list[i - ' '].uv[1] = (fpos + glm::vec2(0, -NUM_CBLOCK)) / (float)fonttex_size;
		//list[i - ' '].uv[2] = (fpos + glm::vec2(NUM_CBLOCK, -NUM_CBLOCK)) / (float)fonttex_size;
		//list[i - ' '].uv[3] = (fpos + glm::vec2(NUM_CBLOCK, 0)) / (float)fonttex_size;
		list[i - ' '].xy = fpos / (float)fonttex_size;
		list[i - ' '].wh =
			(fpos + glm::vec2(font_size, -font_size)) / (float)fonttex_size;
	}
	glFlush();

	std::vector<unsigned char> buf;
	buf.resize(tex_size * tex_size);
	glGetTextureImage(temp_texture, 0, GL_RED, GL_UNSIGNED_BYTE,
					  (int)buf.size(), buf.data());
	cv::Mat img(cv::Size(tex_size, tex_size), CV_8UC1, buf.data());
	cv::imwrite("tv_font_texture.png", img);
	cv::Mat flip_img(cv::Size(tex_size, tex_size), CV_8UC1);
	cv::flip(img, flip_img, 0);

	glDeleteTextures(1, &temp_texture);
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glCreateSamplers(1, &sampler);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureImage2DEXT(texture, GL_TEXTURE_2D, 0, GL_COMPRESSED_RED, tex_size,
						tex_size, 0, GL_RED, GL_UNSIGNED_BYTE, flip_img.data);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFlush();

	program.SetProgram(FONT_VERTEX_SHADER, FONT_FRAGMENT_SHADER);
}

void glslStringDraw::Set(int x, int y, const std::string& str) {
	glm::vec2 screen_trans((1.f / (float)window_size.x) * 2.f,
						   (1.f / (float)window_size.y) * 2.f);
	glm::vec2 screen_coord((float)x * screen_trans.x - 1.f,
						   float(window_size.y - y) * screen_trans.y - 1.f);
	float     offset(0);
	for (int i = 0; i < str.length(); ++i) {
		int               code = str[i] - ' ';
		const CharctorUV& uv   = list[(code < 0 ? 0 : code)];
		buffer.emplace_back(screen_coord.x + offset, screen_coord.y);
		buffer.emplace_back(uv.xy);
		buffer.emplace_back(screen_coord.x + offset,
							screen_coord.y - font_size * screen_trans.y);
		buffer.emplace_back(uv.xy.x, uv.wh.y);
		buffer.emplace_back(screen_coord.x + offset +
								font_size * screen_trans.x,
							screen_coord.y - font_size * screen_trans.y);
		buffer.emplace_back(uv.wh);
		buffer.emplace_back(screen_coord.x + offset +
								font_size * screen_trans.x,
							screen_coord.y);
		buffer.emplace_back(uv.wh.x, uv.xy.y);
		element.emplace_back(0 + element_offset);
		element.emplace_back(1 + element_offset);
		element.emplace_back(2 + element_offset);
		element.emplace_back(0 + element_offset);
		element.emplace_back(2 + element_offset);
		element.emplace_back(3 + element_offset);
		offset += (font_size + 4) / (float)window_size.x;
		element_offset += 4;
	}
}

void glslStringDraw::Draw() {
	//buffer.emplace_back(-0.9f, 0.9f);	buffer.emplace_back(0.f, 1.f);
	//buffer.emplace_back(-0.9f, 0.4f);	buffer.emplace_back(0.f, 0.f);
	//buffer.emplace_back(-0.6f, 0.4f);	buffer.emplace_back(1.f, 0.f);
	//buffer.emplace_back(-0.6f, 0.9f);	buffer.emplace_back(1.f, 1.f);
	//element.emplace_back(0 + element_offset);
	//element.emplace_back(1 + element_offset);
	//element.emplace_back(2 + element_offset);
	//element.emplace_back(0 + element_offset);
	//element.emplace_back(2 + element_offset);
	//element.emplace_back(3 + element_offset);
	if (element.empty())
		return;
	GLuint pos = program.GetAttrib("position")->GetIndex(),
		   uv  = program.GetAttrib("uv_coord")->GetIndex();
	if (!vao)
		glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glCreateBuffers(2, &vbo);
	glVertexArrayElementBuffer(vao, ebo);
	glNamedBufferData(ebo, element.size() * sizeof(unsigned int),
					  element.data(), GL_STREAM_DRAW);
	glNamedBufferData(vbo, buffer.size() * sizeof(glm::vec2), buffer.data(),
					  GL_STREAM_DRAW);
	glVertexArrayVertexBuffer(vao, 1, vbo, 0, sizeof(glm::vec2) * 2);
	glVertexArrayAttribFormat(vao, pos, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(vao, uv, 2, GL_FLOAT, GL_FALSE,
							  sizeof(glm::vec2));
	glVertexArrayAttribBinding(vao, pos, 1);
	glVertexArrayAttribBinding(vao, uv, 1);
	glEnableVertexArrayAttrib(vao, pos);
	glEnableVertexArrayAttrib(vao, uv);
	glBindTextureUnit(8, texture);
	glBindSampler(8, sampler);

	//glDisable(GL_CULL_FACE);
	GLuint prog = program.GetProgram();
	glUseProgram(prog);
	glProgramUniform1i(prog, program.GetUniform("texture")->GetIndex(), 8);
	glProgramUniform3f(prog, program.GetUniform("font_color")->GetIndex(), 1.f,
					   1.f, 1.f);
	glDrawElements(GL_TRIANGLES, (int)element.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
	glUseProgram(0);
	//glEnable(GL_CULL_FACE);

	glVertexArrayElementBuffer(vao, 0);
	glVertexArrayVertexBuffer(vao, 0, 0, 0, 0);
	glDeleteBuffers(2, &vbo);
	//glDeleteVertexArrays(1, &vao);
	glFlush();

	buffer.clear();
	element.clear();
	element_offset = 0;
}

void glslStringDraw::SetWindowSize(int w, int h) {
	window_size = glm::ivec2(w, h);
}

glslStringDraw glslStringDraw::stringDraw;

} // namespace tv
