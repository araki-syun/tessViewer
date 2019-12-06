#include "glslStringDraw.h"

#include <filesystem>
#include <opencv2\core.hpp>
#include <opencv2\imgcodecs.hpp>
#include <opencv2\highgui.hpp>

#include "define.h"
#include "log.h"

#define NUM_CBLOCK 16

namespace tv {
GlslStringDraw::GlslStringDraw() = default;

GlslStringDraw::~GlslStringDraw() {
	FT_Done_Face(_ftface);
	FT_Done_FreeType(_ftlib);
	if (_texture != 0u) {
		glDeleteTextures(1, &_texture);
	}
	if (_sampler != 0u) {
		glDeleteSamplers(1, &_sampler);
	}
	if (_vao != 0u) {
		glDeleteVertexArrays(1, &_vao);
	}
}

GlslStringDraw* GlslStringDraw::GetInstance() { return &stringDraw; }

void GlslStringDraw::Initialize(int                          fontsize,
								const std::filesystem::path& filepath) {
	fontsize = fontsize;
	if (FT_Init_FreeType(&_ftlib) != 0) {
		throw GraphicsError(LogLevel::Error, "ERROR : freetype Initialize");
	}
	if (std::filesystem::exists(filepath)) {
		if (FT_New_Face(_ftlib, filepath.generic_string().c_str(), 0,
						&_ftface) != 0) {
			throw AppError(LogLevel::Error, "ERROR : font file Load\n");
		}
	} else {
		throw AppError(LogLevel::Error, "ERROR : font file Not Found\n");
	}
	if (FT_Set_Char_Size(_ftface, 0, _font_size * 64, 80, 80) != 0) {
		throw GraphicsError(LogLevel::Error, "ERROR : set font char size\n");
	}
	//if(FT_Set_Pixel_Sizes(this->_face, w, h))
	//	throw std::exception("ERROR : font set pixel\n");

	_fonttex_size         = _font_size * NUM_CBLOCK;
	unsigned int tex_size = 1;
	while (tex_size < (unsigned int)_fonttex_size) {
		tex_size <<= 1;
	}
	GLuint temp_texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &temp_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureImage2DEXT(temp_texture, GL_TEXTURE_2D, 0, GL_RED, tex_size,
						tex_size, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	_list.resize('~' - ' ');
	for (int i = ' '; i < '~'; ++i) {
		FT_UInt index = FT_Get_Char_Index(_ftface, i);
		if (FT_Load_Char(_ftface, i, FT_LOAD_DEFAULT) != 0) {
			throw InterfaceError(LogLevel::Error,
								 fmt::format("Load Char : {}", (char)i));
		}
		if (FT_Render_Glyph(_ftface->glyph,
							FT_Render_Mode::FT_RENDER_MODE_NORMAL) != 0) {
			throw InterfaceError(LogLevel::Error, "Render Gliph");
		}
		FT_Bitmap& bm = _ftface->glyph->bitmap;

		glm::ivec2 pos((i & 0x0F) * (_fonttex_size / _font_size),
					   ((i & 0xF0) >> 4) * (_fonttex_size / _font_size));

		glTextureSubImage2D(
			temp_texture, 0, pos.x + _ftface->glyph->metrics.horiBearingX / 64,
			pos.y + _font_size -
				_ftface->glyph->metrics.horiBearingY / 64, //- bm->top_margin,
			bm.width, bm.rows, GL_RED, GL_UNSIGNED_BYTE, bm.buffer);
		glm::vec2 fpos(pos.x, tex_size - pos.y);
		//list[i - ' '].uv[0] = fpos / (float)fonttex_size;
		//list[i - ' '].uv[1] = (fpos + glm::vec2(0, -NUM_CBLOCK)) / (float)fonttex_size;
		//list[i - ' '].uv[2] = (fpos + glm::vec2(NUM_CBLOCK, -NUM_CBLOCK)) / (float)fonttex_size;
		//list[i - ' '].uv[3] = (fpos + glm::vec2(NUM_CBLOCK, 0)) / (float)fonttex_size;
		_list[i - ' '].xy = fpos / (float)_fonttex_size;
		_list[i - ' '].wh =
			(fpos + glm::vec2(_font_size, -_font_size)) / (float)_fonttex_size;
	}
	glFlush();

	std::vector<unsigned char> buf;
	buf.resize(tex_size * tex_size);
	glGetTextureImage(temp_texture, 0, GL_RED, GL_UNSIGNED_BYTE,
					  (int)buf.size(), buf.data());
	cv::Mat img(cv::Size(tex_size, tex_size), CV_8UC1, buf.data());
#ifdef EXPORT_FONT_TEXTURE
	cv::imwrite("tv_font_texture.png", img);
#endif
	cv::Mat flip_img(cv::Size(tex_size, tex_size), CV_8UC1);
	cv::flip(img, flip_img, 0);

	glDeleteTextures(1, &temp_texture);
	glCreateTextures(GL_TEXTURE_2D, 1, &_texture);
	glCreateSamplers(1, &_sampler);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureImage2DEXT(_texture, GL_TEXTURE_2D, 0, GL_COMPRESSED_RED, tex_size,
						tex_size, 0, GL_RED, GL_UNSIGNED_BYTE, flip_img.data);
	glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFlush();

	_program.SetProgram(FONT_VERTEX_SHADER, FONT_FRAGMENT_SHADER);
}

void GlslStringDraw::Set(int x, int y, const std::string& str) {
	glm::vec2 screen_trans((1.f / (float)_window_size.x) * 2.f,
						   (1.f / (float)_window_size.y) * 2.f);
	glm::vec2 screen_coord((float)x * screen_trans.x - 1.f,
						   float(_window_size.y - y) * screen_trans.y - 1.f);
	float     offset(0);
	auto      size = float(_font_size);
	for (char i : str) {
		int               code = i - ' ';
		const CharctorUV& uv   = _list[(code < 0 ? 0 : code)];
		_buffer.emplace_back(screen_coord.x + offset, screen_coord.y);
		_buffer.emplace_back(uv.xy);
		_buffer.emplace_back(screen_coord.x + offset,
							 screen_coord.y - size * screen_trans.y);
		_buffer.emplace_back(uv.xy.x, uv.wh.y);
		_buffer.emplace_back(screen_coord.x + offset + size * screen_trans.x,
							 screen_coord.y - size * screen_trans.y);
		_buffer.emplace_back(uv.wh);
		_buffer.emplace_back(screen_coord.x + offset + size * screen_trans.x,
							 screen_coord.y);
		_buffer.emplace_back(uv.wh.x, uv.xy.y);
		_element.emplace_back(0 + _element_offset);
		_element.emplace_back(1 + _element_offset);
		_element.emplace_back(2 + _element_offset);
		_element.emplace_back(0 + _element_offset);
		_element.emplace_back(2 + _element_offset);
		_element.emplace_back(3 + _element_offset);
		offset += (size + 4) / (float)_window_size.x;
		_element_offset += 4;
	}
}

void GlslStringDraw::Draw() {
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
	if (_element.empty()) {
		return;
	}
	GLuint pos = _program.GetAttrib("position")->GetIndex();
	GLuint uv  = _program.GetAttrib("uv_coord")->GetIndex();
	if (_vao == 0u) {
		glCreateVertexArrays(1, &_vao);
	}
	glBindVertexArray(_vao);
	glCreateBuffers(2, &_vbo);
	glVertexArrayElementBuffer(_vao, _ebo);
	glNamedBufferData(_ebo, _element.size() * sizeof(unsigned int),
					  _element.data(), GL_STREAM_DRAW);
	glNamedBufferData(_vbo, _buffer.size() * sizeof(glm::vec2), _buffer.data(),
					  GL_STREAM_DRAW);
	glVertexArrayVertexBuffer(_vao, 1, _vbo, 0, sizeof(glm::vec2) * 2);
	glVertexArrayAttribFormat(_vao, pos, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(_vao, uv, 2, GL_FLOAT, GL_FALSE,
							  sizeof(glm::vec2));
	glVertexArrayAttribBinding(_vao, pos, 1);
	glVertexArrayAttribBinding(_vao, uv, 1);
	glEnableVertexArrayAttrib(_vao, pos);
	glEnableVertexArrayAttrib(_vao, uv);
	glBindTextureUnit(8, _texture);
	glBindSampler(8, _sampler);

	//glDisable(GL_CULL_FACE);
	GLuint prog = _program.GetProgram();
	glUseProgram(prog);
	glProgramUniform1i(prog, _program.GetUniform("texture")->GetIndex(), 8);
	glProgramUniform3f(prog, _program.GetUniform("font_color")->GetIndex(), 1.f,
					   1.f, 1.f);
	glDrawElements(GL_TRIANGLES, (int)_element.size(), GL_UNSIGNED_INT,
				   nullptr);
	glBindVertexArray(0);
	glUseProgram(0);
	//glEnable(GL_CULL_FACE);

	glVertexArrayElementBuffer(_vao, 0);
	glVertexArrayVertexBuffer(_vao, 0, 0, 0, 0);
	glDeleteBuffers(2, &_vbo);
	//glDeleteVertexArrays(1, &vao);
	glFlush();

	_buffer.clear();
	_element.clear();
	_element_offset = 0;
}

void GlslStringDraw::SetWindowSize(int w, int h) {
	_window_size = glm::ivec2(w, h);
}

GlslStringDraw GlslStringDraw::stringDraw;

} // namespace tv
