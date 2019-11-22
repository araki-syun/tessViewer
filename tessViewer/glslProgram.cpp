#include "glslProgram.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include <fmt\format.h>

GLuint CompileShader(GLenum type, std::string& src) {
	GLuint      shader(glCreateShader(type));
	const char* c_src = src.c_str();
	glShaderSource(shader, 1, &c_src, nullptr);
	glCompileShader(shader);
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE) {
		GLint log_size;
		GLint length;

		/* ログの長さは、最後のNULL文字も含む */
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
		std::vector<GLchar> log(log_size);

		if (log_size > 1) {
			glGetShaderInfoLog(shader, log_size, &length, log.data());
			std::cerr << log.data() << std::endl;
		}
		glDeleteShader(shader);
		std::string shader_type;
		switch (type) {
		case GL_VERTEX_SHADER: shader_type = "Vertex"; break;
		case GL_FRAGMENT_SHADER: shader_type = "Fragment"; break;
		case GL_GEOMETRY_SHADER: shader_type = "Geometry"; break;
		case GL_TESS_EVALUATION_SHADER: shader_type = "Tess_Evaluation"; break;
		case GL_TESS_CONTROL_SHADER: shader_type = "Tess_Control"; break;
		default: break;
		}
		throw std::runtime_error(fmt::format(
			"GLSL {} Shader Compile ERROR\n{}\n", shader_type, log.data()));
	}
	return shader;
}

GlslProgram::GlslProgram() = default;
GlslProgram::GlslProgram(const GlslInfo& glsl) { SetProgram(glsl); }
GlslProgram::GlslProgram(const GlslInfo& glsl, const OsdInfo& osd) {
	SetProgram(glsl, osd);
}
GlslProgram::~GlslProgram() {
	if (_program != 0u) {
		glDeleteProgram(_program);
	}
}

GLuint GlslProgram::GetProgram() const { return _program; }

void GlslProgram::SetProgram(const GlslProgram::GlslInfo& glsl) {
	if (_program != 0u) {
		glDeleteProgram(_program);
	}
	_program = glCreateProgram();

	std::ifstream ifsc(SHADER "common.glsl", std::ios::in);
	std::ifstream ifsv(glsl.vert, std::ios::in);
	std::ifstream ifsf(glsl.frag, std::ios::in);
	std::ifstream ifsg(glsl.geom, std::ios::in);
	std::ifstream ifstc(glsl.tcs, std::ios::in);
	std::ifstream ifste(glsl.tes, std::ios::in);

	// vertex shader
	{
		if (ifsv.fail()) {
			ifsv.close();
			ifsv.open(DEFAULT_VERTEX_SHADER, std::ios::in);
		}
		std::string str((std::istreambuf_iterator<char>(ifsv)),
						std::istreambuf_iterator<char>());
		GLuint      shader = CompileShader(GL_VERTEX_SHADER, str);
		glAttachShader(_program, shader);
		glDeleteShader(shader);
	}
	// fragment shader
	{
		if (ifsf.fail()) {
			ifsf.close();
			ifsf.open(DEFAULT_FRAGMENT_SHADER, std::ios::in);
		}
		std::string str((std::istreambuf_iterator<char>(ifsf)),
						std::istreambuf_iterator<char>());
		GLuint      shader = CompileShader(GL_FRAGMENT_SHADER, str);
		glAttachShader(_program, shader);
		glDeleteShader(shader);
	}
	// geometry shader
	{
		if (ifsg) {
			std::string str((std::istreambuf_iterator<char>(ifsg)),
							std::istreambuf_iterator<char>());
			GLuint      shader = CompileShader(GL_GEOMETRY_SHADER, str);
			glAttachShader(_program, shader);
			glDeleteShader(shader);
		}
	}
	// tesselation control shader
	{
		if (ifsg) {
			std::string str((std::istreambuf_iterator<char>(ifsc)),
							std::istreambuf_iterator<char>());
			GLuint      shader = CompileShader(GL_TESS_CONTROL_SHADER, str);
			glAttachShader(_program, shader);
			glDeleteShader(shader);
		}
	}
	// tesselation evalution shader
	{
		if (ifsg) {
			std::string str((std::istreambuf_iterator<char>(ifste)),
							std::istreambuf_iterator<char>());
			GLuint      shader = CompileShader(GL_TESS_EVALUATION_SHADER, str);
			glAttachShader(_program, shader);
			glDeleteShader(shader);
		}
	}
	GLint linked;
	glLinkProgram(_program);
	glGetProgramiv(_program, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE) {
		GLint size;
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &size);
		std::vector<GLchar> log(size);
		if (size > 1) {
			GLint length;
			glGetProgramInfoLog(_program, size, &length, log.data());
		}
		glDeleteProgram(_program);
		_program        = 0;
		std::string err = "GLSL Program Link ERROR\n";
		throw std::runtime_error(err.append(log.data()));
	}
	_set_location();
}

void GlslProgram::SetProgram(const GlslProgram::GlslInfo& glsl,
							 const OsdInfo&               osd) {
	using namespace OpenSubdiv;
	if (_program != 0u) {
		glDeleteProgram(_program);
	}
	_program = glCreateProgram();

	std::ifstream ifsc(SHADER "common.glsl", std::ios::in);
	std::ifstream ifsv(glsl.vert, std::ios::in);
	std::ifstream ifsf(glsl.frag, std::ios::in);
	std::ifstream ifsg(glsl.geom, std::ios::in);
	std::ifstream ifstc(glsl.tcs, std::ios::in);
	std::ifstream ifste(glsl.tes, std::ios::in);

	std::string osd_def((std::istreambuf_iterator<char>(ifsc)),
						std::istreambuf_iterator<char>());

	std::stringstream ss;

	ss << GLSL_VERSION << "#define PRIM_TRI\n"
	   << (osd.elem.bits.screen_space_tess
			   ? "#define OSD_ENABLE_SCREENSPACE_TESSELLATION\n"
			   : "")
	   << (osd.elem.bits.fractional ? "#define OSD_FRACTIONAL_EVEN_SPACING\n"
									: "")
	   << (osd.elem.bits.patch_cull ? "#define OSD_ENABLE_PATCH_CULL\n" : "")
	   << (osd.elem.bits.patch_type == Far::PatchDescriptor::Type::GREGORY_BASIS
			   ? ""
			   : (osd.elem.bits.single_crease_patch
					  ? "#define OSD_PATCH_ENABLE_SINGLE_CREASE\n"
					  : ""))
	   << (osd.elem.bits.fvar_width != 0u
			   ? fmt::format("#define OSD_FVAR_WIDTH {}\n",
							 osd.elem.bits.fvar_width)

			   : "");

	ss << Osd::GLSLPatchShaderSource::GetCommonShaderSource();
	std::string common = ss.str();
	ss.str("");

	std::string shader_src[5];

	// vertex shader
	{
		if (ifsv.fail()) {
			ifsv.close();
			ifsv.open(DEFAULT_VERTEX_SHADER, std::ios::in);
		}
		std::string str((std::istreambuf_iterator<char>(ifsv)),
						std::istreambuf_iterator<char>());
		ss << common << osd_def << "#define VERTEX_SHADER\n"
		   << str
		   << Osd::GLSLPatchShaderSource::GetVertexShaderSource(
				  osd.elem.GetPatchType());
		shader_src[0] = ss.str();
		std::ofstream ofs(SHADER "gen.vert", std::ios::trunc);
		ofs << shader_src[0] << std::endl;
		GLuint shader = CompileShader(GL_VERTEX_SHADER, shader_src[0]);
		glAttachShader(_program, shader);
		glDeleteShader(shader);
		ss.str("");
	}
	// fragment shader
	{
		if (ifsf.fail()) {
			ifsf.close();
			ifsf.open(DEFAULT_FRAGMENT_SHADER, std::ios::in);
		}
		std::string str((std::istreambuf_iterator<char>(ifsf)),
						std::istreambuf_iterator<char>());
		ss << common << osd_def << "#define FRAGMENT_SHADER\n" << str;
		shader_src[1] = ss.str();
		std::ofstream ofs(SHADER "gen.frag", std::ios::trunc);
		ofs << shader_src[1] << std::endl;
		GLuint shader = CompileShader(GL_FRAGMENT_SHADER, shader_src[1]);
		glAttachShader(_program, shader);
		glDeleteShader(shader);
		ss.str("");
	}
	// geometry shader
	if (ifsg) {
		std::string str((std::istreambuf_iterator<char>(ifsg)),
						std::istreambuf_iterator<char>());
		ss << common << osd_def << "#define GEOMETRY_SHADER\n" << str;
		shader_src[2] = ss.str();
		std::ofstream ofs(SHADER "gen.geom", std::ios::trunc);
		ofs << shader_src[2] << std::endl;
		GLuint shader = CompileShader(GL_GEOMETRY_SHADER, shader_src[2]);
		glAttachShader(_program, shader);
		glDeleteShader(shader);
		ss.str("");
	}
	// tesslation evalusion shader
	{
		ss << common << osd_def;
		if (ifste) {
			std::string str((std::istreambuf_iterator<char>(ifste)),
							std::istreambuf_iterator<char>());
			ss << str;
		}
		ss << Osd::GLSLPatchShaderSource::GetTessEvalShaderSource(
			osd.elem.GetPatchType());
		shader_src[3] = ss.str();
		std::ofstream ofs(SHADER "gen.tese", std::ios::trunc);
		ofs << shader_src[3] << std::endl;
		GLuint shader = CompileShader(GL_TESS_EVALUATION_SHADER, shader_src[3]);
		glAttachShader(_program, shader);
		glDeleteShader(shader);
		ss.str("");
	}
	// tesslation controll shader
	{
		ss << common << osd_def;
		if (ifste) {
			std::string str((std::istreambuf_iterator<char>(ifstc)),
							std::istreambuf_iterator<char>());
		}
		ss << Osd::GLSLPatchShaderSource::GetTessControlShaderSource(
			osd.elem.GetPatchType());
		shader_src[4] = ss.str();
		std::ofstream ofs(SHADER "gen.tesc", std::ios::trunc);
		ofs << shader_src[4] << std::endl;
		GLuint shader = CompileShader(GL_TESS_CONTROL_SHADER, shader_src[4]);
		glAttachShader(_program, shader);
		glDeleteShader(shader);
		ss.str("");
	}
	GLint linked;
	glLinkProgram(_program);
	glGetProgramiv(_program, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE) {
		GLint size;
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &size);
		std::vector<GLchar> log(size);
		if (size > 1) {
			GLint length;
			glGetProgramInfoLog(_program, size, &length, log.data());
		}
		glDeleteProgram(_program);
		_program        = 0;
		std::string err = "GLSL Program Link ERROR\n";
		throw std::runtime_error(err.append(log.data()));
	}
	_set_location();
}

void GlslProgram::SetProgram(const std::string& vert, const std::string& frag) {
	if (_program != 0u) {
		glDeleteProgram(_program);
	}
	_program = glCreateProgram();

	std::ifstream ifsv(vert, std::ios::in);
	std::ifstream ifsf(frag, std::ios::in);

	// vertex shader
	{
		if (ifsv.fail()) {
			throw std::runtime_error(
				fmt::format("Font Vertex Shader failed to open : {}", vert));
		}
		std::string str((std::istreambuf_iterator<char>(ifsv)),
						std::istreambuf_iterator<char>());
		GLuint      shader = CompileShader(GL_VERTEX_SHADER, str);
		glAttachShader(_program, shader);
		glDeleteShader(shader);
	}
	// fragment shader
	{
		if (ifsf.fail()) {
			throw std::runtime_error(
				fmt::format("Font Fragment Shader failed to open : {}", frag));
		}
		std::string str((std::istreambuf_iterator<char>(ifsf)),
						std::istreambuf_iterator<char>());
		GLuint      shader = CompileShader(GL_FRAGMENT_SHADER, str);
		glAttachShader(_program, shader);
		glDeleteShader(shader);
	}
	GLint linked;
	glLinkProgram(_program);
	glGetProgramiv(_program, GL_LINK_STATUS, &linked);
	if (linked != GL_TRUE) {
		GLint size;
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &size);
		std::vector<GLchar> log(size);
		if (size > 1) {
			GLint length;
			glGetProgramInfoLog(_program, size, &length, log.data());
		}
		glDeleteProgram(_program);
		_program        = 0;
		std::string err = "GLSL Program Link ERROR\n";
		throw std::runtime_error(err.append(log.data()));
	}
	_set_location();
}

void GlslProgram::_set_location() {

	{
		_attrib_map.clear();
		int num_attrib;
		int max_attribsize;
		glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &num_attrib);
		glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,
					   &max_attribsize);
		for (int i = 0; i < num_attrib; ++i) {
			int               size;
			GLenum            type;
			std::vector<char> name(max_attribsize);
			glGetActiveAttrib(_program, i, max_attribsize, nullptr, &size,
							  &type, name.data());
			_attrib_map[name.data()] =
				std::make_unique<tv::GlShaderAttribute>(_program, name.data());
		}
	}
	{
		_uniform_map.clear();
		int num_uniform;
		int max_uniformsize;
		glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &num_uniform);
		glGetProgramiv(_program, GL_ACTIVE_UNIFORM_MAX_LENGTH,
					   &max_uniformsize);
		for (int i = 0; i < num_uniform; ++i) {
			std::vector<char> name(max_uniformsize);
			glGetActiveUniformName(_program, i, max_uniformsize, nullptr,
								   name.data());
			_uniform_map[name.data()] =
				std::make_unique<tv::GlShaderUniform>(_program, name.data());
		}
	}
	{
		_uniform_block_map.clear();
		int num_uniform_block;
		int max_uniform_blocksize;
		glGetProgramiv(_program, GL_ACTIVE_UNIFORM_BLOCKS, &num_uniform_block);
		glGetProgramiv(_program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH,
					   &max_uniform_blocksize);
		for (int i = 0; i < num_uniform_block; ++i) {
			int               length;
			int               size;
			std::vector<char> name_array(max_uniform_blocksize);
			glGetActiveUniformBlockName(_program, i, max_uniform_blocksize,
										&length, name_array.data());
			glGetActiveUniformBlockiv(_program, i, GL_UNIFORM_BLOCK_DATA_SIZE,
									  &size);
			std::string name(name_array.data()); //cbegin(), name_array.cend());
			_uniform_block_map[name] =
				std::make_unique<tv::GlShaderUniformBlock>(_program,
														   name.c_str());
			glUniformBlockBinding(
				_program, _uniform_block_map[name]->GetIndex(),
				GlslInfo::GetUniformBuffer(name)->GetBinding());
		}
	}
	_fragment = glGetFragDataLocation(_program, "fragment");
}

const tv::GlShaderAttribute*
GlslProgram::GetAttrib(const std::string& name) const {
	return _attrib_map.at(name).get();
}

const tv::GlShaderUniform*
GlslProgram::GetUniform(const std::string& name) const {
	return _uniform_map.at(name).get();
}

const tv::GlShaderUniformBlock*
GlslProgram::GetUniformBlock(const std::string& name) const {
	return _uniform_block_map.at(name).get();
}

//GlslProgram::GlslInfo::GlslInfo() :
//	vert(DEFAULT_VERTEX_SHADER),
//	frag(DEFAULT_FRAGMENT_SHADER),
//	geom(DEFAULT_GEOMETRY_SHADER)
//{
//	elem.bits.fvar_width=0;
//	elem.bits.patch_type = OpenSubdiv::Far::PatchDescriptor::Type::NON_PATCH;
//	elem.bits.num_prim_per_vertex = 3;
//	elem.bits.is_adaptive = 1;
//	elem.bits.screen_space_tess = 1;
//	elem.bits.fractional = 0;
//	elem.bits.patch_cull = 0;
//	elem.bits.single_crease_patch = 0;
//}
//
//GlslProgram::GlslInfo::GlslInfo(const GlslInfo & info)
//{
//}
//
//GlslProgram::GlslInfo::GlslInfo(
//	const std::string & vert,
//	const std::string & frag,
//	const std::string & geom,
//	const std::string & tcs,
//	const std::string & tes):
//	vert(vert),frag(frag),geom(geom),tcs(tcs),tes(tes)
//{
//}
//
//void GlslProgram::GlslInfo::set(Elem_Type e, unsigned int value)
//{
//	switch (e)
//	{
//	case GlslProgram::GlslInfo::FVAR_WIDTH:
//		elem.bits.fvar_width = value;
//		break;
//	case GlslProgram::GlslInfo::PATCH_TYPE:
//		elem.bits.patch_type = value;
//		break;
//	case GlslProgram::GlslInfo::NUM_PRIM_PER_VERTEX:
//		elem.bits.num_prim_per_vertex = value;
//		break;
//	case GlslProgram::OsdInfo::IS_ADAPTIVE:
//		elem.bits.is_adaptive = (value ? 1 : 0);
//		break;
//	case GlslProgram::OsdInfo::SCREEN_SPACE_TESS:
//		elem.bits.screen_space_tess = (value ? 1 : 0);
//		break;
//	case GlslProgram::OsdInfo::FRACTIONAL:
//		elem.bits.fractional = (value ? 1 : 0);
//		break;
//	case GlslProgram::OsdInfo::PATCH_CULL:
//		elem.bits.patch_cull = (value ? 1 : 0);
//		break;
//	case GlslProgram::OsdInfo::SINGLE_CREASE_PATCH:
//		elem.bits.single_crease_patch = (value ? 1 : 0);
//		break;
//	default:
//		break;
//	}
//}
//
//const std::string GlslProgram::GlslInfo::str() const
//{
//	std::stringstream ss;
//	//std::stringstream::fmtflags flag = ss.flags();
//	ss << vert << frag << geom << tcs << tes;
//	return ss.str();
//}
//
//bool GlslProgram::OsdInfo::Element::operator==(const GlslProgram::OsdInfo::Element e) const
//{
//	return data == e.data;
//}
//
//OpenSubdiv::Far::PatchDescriptor::Type GlslProgram::OsdInfo::Element::Get_patch_type() const
//{
//	return OpenSubdiv::Far::PatchDescriptor::Type(bits.patch_type);
//}
//
//void GlslProgram::OsdInfo::Element::Set_patch_type(OpenSubdiv::Far::PatchDescriptor::Type type)
//{
//	bits.patch_type = type;
//}

GlslProgram::GlslInfo::GlslInfo() /* :
	vert(DEFAULT_VERTEX_SHADER),frag(DEFAULT_FRAGMENT_SHADER),geom(DEFAULT_GEOMETRY_SHADER)*/
	= default;

// GlslProgram::GlslInfo::GlslInfo(const GlslInfo& info)
// 	: vert(info.vert)
// 	, frag(info.vert)
// 	, geom(info.vert)
// 	, tcs(info.vert)
// 	, tes(info.vert) {}

GlslProgram::GlslInfo::GlslInfo(std::string vert,
								std::string frag,
								std::string geom,
								std::string tcs,
								std::string tes)
	: vert(std::move(vert))
	, frag(std::move(frag))
	, geom(std::move(geom))
	, tcs(std::move(tcs))
	, tes(std::move(tes)) {}

std::string GlslProgram::GlslInfo::Str() const {
	std::stringstream ss;
	ss << vert << frag << geom << tcs << tes;
	return ss.str();
}

void GlslProgram::GlslInfo::CreateUniformBuffer(const std::string& name,
												int                size) {
	auto it = buffers.find(name);
	if (it != buffers.end()) {
		;
	} else {
		buffers.emplace(
			std::piecewise_construct, std::forward_as_tuple(name),
			std::forward_as_tuple(new tv::GlShaderUniformBuffer(size)));
	}
}

const tv::GlShaderUniformBuffer*
GlslProgram::GlslInfo::GetUniformBuffer(const std::string& name) {
	try {
		return buffers.at(name).get();
	}
	catch (std::out_of_range& e) {
		std::cout << name << " は存在しません" << std::endl;
		throw;
	}
}

std::map<std::string, std::unique_ptr<tv::GlShaderUniformBuffer>>
	GlslProgram::GlslInfo::buffers;
