#include "glslProgram.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <boost\format.hpp>

GLuint CompileShader(GLenum type, std::string& src) {
	GLuint shader(glCreateShader(type));
	const char* c_src = src.c_str();
	glShaderSource(shader, 1, &c_src, nullptr);
	glCompileShader(shader);
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE) {
		GLint logSize;
		GLint length;

		/* ÉçÉOÇÃí∑Ç≥ÇÕÅAç≈å„ÇÃNULLï∂éöÇ‡ä‹Çﬁ */
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		std::vector<GLchar> log(logSize);

		if (logSize > 1)
		{
			glGetShaderInfoLog(shader, logSize,
				&length, log.data());
			std::cerr << log.data() << std::endl;
		}
		glDeleteShader(shader);
		std::string shader_type;
		switch (type)
		{
		case GL_VERTEX_SHADER:	shader_type = "Vertex"; break;
		case GL_FRAGMENT_SHADER:	shader_type = "Fragment"; break;
		case GL_GEOMETRY_SHADER:	shader_type = "Geometry"; break;
		case GL_TESS_EVALUATION_SHADER:	shader_type = "Tess_Evaluation"; break;
		case GL_TESS_CONTROL_SHADER:	shader_type = "Tess_Control"; break;
		default:
			break;
		}
		throw std::runtime_error((
			boost::format("GLSL %1% Shader Compile ERROR\n%2%\n")
			% shader_type
			% log.data()
			).str());
	}
	return shader;
}

glslProgram::glslProgram() :
	_program(0)
{
}
glslProgram::glslProgram(const glsl_info & glsl)
{
	SetProgram(glsl);
}
glslProgram::glslProgram(const glsl_info & glsl, const osd_info & osd)
{
	SetProgram(glsl, osd);
}
glslProgram::~glslProgram()
{
	if (_program)
		glDeleteProgram(_program);
}

GLuint glslProgram::GetProgram() const
{
	return _program;
}

void glslProgram::SetProgram(const glslProgram::glsl_info& glsl)
{
	if (_program)
		glDeleteProgram(_program);
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
		GLuint shader = CompileShader(GL_VERTEX_SHADER, str);
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
		GLuint shader = CompileShader(GL_FRAGMENT_SHADER, str);
		glAttachShader(_program, shader);
		glDeleteShader(shader);
	}
	// geometry shader
	{
		if (ifsg) {
			std::string str((std::istreambuf_iterator<char>(ifsg)),
				std::istreambuf_iterator<char>());
			GLuint shader = CompileShader(GL_GEOMETRY_SHADER, str);
			glAttachShader(_program, shader);
			glDeleteShader(shader);
		}
	}
	// tesselation control shader
	{
		if (ifsg) {
			std::string str((std::istreambuf_iterator<char>(ifsc)),
				std::istreambuf_iterator<char>());
			GLuint shader = CompileShader(GL_TESS_CONTROL_SHADER, str);
			glAttachShader(_program, shader);
			glDeleteShader(shader);
		}
	}
	// tesselation evalution shader
	{
		if (ifsg) {
			std::string str((std::istreambuf_iterator<char>(ifste)),
				std::istreambuf_iterator<char>());
			GLuint shader = CompileShader(GL_TESS_EVALUATION_SHADER, str);
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
		_program = 0;
		std::string err = "GLSL Program Link ERROR\n";
		throw std::runtime_error(err.append(log.data()));
	}
	SetLocation();
}

void glslProgram::SetProgram(const glslProgram::glsl_info& glsl, const osd_info& osd)
{
	using namespace OpenSubdiv;
	if (_program)
		glDeleteProgram(_program);
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

	ss << GLSL_VERSION
	 << "#define PRIM_TRI\n"
	 << (osd.elem.bits.screen_space_tess ? "#define OSD_ENABLE_SCREENSPACE_TESSELLATION\n" : "")
	 << (osd.elem.bits.fractional ? "#define OSD_FRACTIONAL_EVEN_SPACING\n" : "")
	 << (osd.elem.bits.patch_cull ? "#define OSD_ENABLE_PATCH_CULL\n" : "")
	 << (osd.elem.bits.patch_type == Far::PatchDescriptor::Type::GREGORY_BASIS ? "" :
		 (osd.elem.bits.single_crease_patch ? "#define OSD_PATCH_ENABLE_SINGLE_CREASE\n" : ""))
	 << (osd.elem.bits.fvar_width ? (boost::format("#define OSD_FVAR_WIDTH %1%\n") % osd.elem.bits.fvar_width).str() : "")
		;
	
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
		ss << common
			<< osd_def
			<< "#define VERTEX_SHADER\n"
			<< str
			<< Osd::GLSLPatchShaderSource::GetVertexShaderSource(osd.elem.Get_patch_type());
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
		ss << common
			<< osd_def
			<< "#define FRAGMENT_SHADER\n"
			<< str;
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
		ss << common
			<< osd_def
			<< "#define GEOMETRY_SHADER\n"
			<< str;
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
		ss << common
			<< osd_def;
		if (ifste) {
			std::string str((std::istreambuf_iterator<char>(ifste)),
				std::istreambuf_iterator<char>());
			ss << str;
		}
		ss << Osd::GLSLPatchShaderSource::GetTessEvalShaderSource(osd.elem.Get_patch_type());
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
		ss << common
			<< osd_def;
		if (ifste) {
			std::string str((std::istreambuf_iterator<char>(ifstc)),
				std::istreambuf_iterator<char>());
		}
		ss << Osd::GLSLPatchShaderSource::GetTessControlShaderSource(osd.elem.Get_patch_type());
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
		_program = 0;
		std::string err = "GLSL Program Link ERROR\n";
		throw std::runtime_error(err.append(log.data()));
	}
	SetLocation();
}

void glslProgram::SetProgram(const std::string & vert, const std::string & frag)
{
	if (_program)
		glDeleteProgram(_program);
	_program = glCreateProgram();

	std::ifstream ifsv(vert, std::ios::in);
	std::ifstream ifsf(frag, std::ios::in);

	// vertex shader
	{
		if (ifsv.fail()) {
			throw std::runtime_error((boost::format("Font Vertex Shader failed to open : %1%") % vert).str());
		}
		std::string str((std::istreambuf_iterator<char>(ifsv)),
			std::istreambuf_iterator<char>());
		GLuint shader = CompileShader(GL_VERTEX_SHADER, str);
		glAttachShader(_program, shader);
		glDeleteShader(shader);
	}
	// fragment shader
	{
		if (ifsf.fail()) {
			throw std::runtime_error((boost::format("Font Fragment Shader failed to open : %1%") % frag).str());
		}
		std::string str((std::istreambuf_iterator<char>(ifsf)),
			std::istreambuf_iterator<char>());
		GLuint shader = CompileShader(GL_FRAGMENT_SHADER, str);
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
		_program = 0;
		std::string err = "GLSL Program Link ERROR\n";
		throw std::runtime_error(err.append(log.data()));
	}
	SetLocation();
}

void glslProgram::SetLocation()
{
	
	{
		attrib_map.clear();
		int numAttrib, maxAttribsize;
		glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &numAttrib);
		glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttribsize);
		for (int i = 0; i < numAttrib; ++i) {
			int size;
			GLenum type;
			std::vector<char> name(maxAttribsize);
			glGetActiveAttrib(_program, i, maxAttribsize, NULL, &size, &type, name.data());
			attrib_map[name.data()].reset(new tv::glShaderAttribute(_program, name.data()));
		}
	}
	{
		uniform_map.clear();
		int numUniform, maxUniformsize;
		glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &numUniform);
		glGetProgramiv(_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformsize);
		for (int i = 0; i < numUniform; ++i) {
			std::vector<char> name(maxUniformsize);
			glGetActiveUniformName(_program, i, maxUniformsize, NULL, name.data());
			uniform_map[name.data()].reset(new tv::glShaderUniform(_program, name.data()));
		}
	}
	{
		uniformBlock_map.clear();
		int numUniformBlock, maxUniformBlocksize;
		glGetProgramiv(_program, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlock);
		glGetProgramiv(_program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxUniformBlocksize);
		for (int i = 0; i < numUniformBlock; ++i) {
			int length,size;
			std::vector<char> name_array(maxUniformBlocksize);
			glGetActiveUniformBlockName(_program, i, maxUniformBlocksize, &length, name_array.data());
			glGetActiveUniformBlockiv(_program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
			std::string name(name_array.data());//cbegin(), name_array.cend());
			uniformBlock_map[name].reset(new tv::glShaderUniformBlock(_program, name.c_str()));
			glUniformBlockBinding(_program, uniformBlock_map[name]->GetIndex(), glsl_info::GetUniformBuffer(name)->GetBinding());
		}
	}
	fragment = glGetFragDataLocation(_program, "fragment");
}

const tv::glShaderAttribute * glslProgram::GetAttrib(const std::string & name) const
{
	return attrib_map.at(name).get();
}

const tv::glShaderUniform * glslProgram::GetUniform(const std::string & name) const
{
	return uniform_map.at(name).get();
}

const tv::glShaderUniformBlock * glslProgram::GetUniformBlock(const std::string & name) const
{
	return uniformBlock_map.at(name).get();
}

//glslProgram::glsl_info::glsl_info() :
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
//glslProgram::glsl_info::glsl_info(const glsl_info & info)
//{
//}
//
//glslProgram::glsl_info::glsl_info(
//	const std::string & vert,
//	const std::string & frag,
//	const std::string & geom,
//	const std::string & tcs,
//	const std::string & tes):
//	vert(vert),frag(frag),geom(geom),tcs(tcs),tes(tes)
//{
//}
//
//void glslProgram::glsl_info::set(Elem_Type e, unsigned int value)
//{
//	switch (e)
//	{
//	case glslProgram::glsl_info::FVAR_WIDTH:
//		elem.bits.fvar_width = value;
//		break;
//	case glslProgram::glsl_info::PATCH_TYPE:
//		elem.bits.patch_type = value;
//		break;
//	case glslProgram::glsl_info::NUM_PRIM_PER_VERTEX:
//		elem.bits.num_prim_per_vertex = value;
//		break;
//	case glslProgram::osd_info::IS_ADAPTIVE:
//		elem.bits.is_adaptive = (value ? 1 : 0);
//		break;
//	case glslProgram::osd_info::SCREEN_SPACE_TESS:
//		elem.bits.screen_space_tess = (value ? 1 : 0);
//		break;
//	case glslProgram::osd_info::FRACTIONAL:
//		elem.bits.fractional = (value ? 1 : 0);
//		break;
//	case glslProgram::osd_info::PATCH_CULL:
//		elem.bits.patch_cull = (value ? 1 : 0);
//		break;
//	case glslProgram::osd_info::SINGLE_CREASE_PATCH:
//		elem.bits.single_crease_patch = (value ? 1 : 0);
//		break;
//	default:
//		break;
//	}
//}
//
//const std::string glslProgram::glsl_info::str() const
//{
//	std::stringstream ss;
//	//std::stringstream::fmtflags flag = ss.flags();
//	ss << vert << frag << geom << tcs << tes;
//	return ss.str();
//}
//
//bool glslProgram::osd_info::Element::operator==(const glslProgram::osd_info::Element e) const
//{
//	return data == e.data;
//}
//
//OpenSubdiv::Far::PatchDescriptor::Type glslProgram::osd_info::Element::Get_patch_type() const
//{
//	return OpenSubdiv::Far::PatchDescriptor::Type(bits.patch_type);
//}
//
//void glslProgram::osd_info::Element::Set_patch_type(OpenSubdiv::Far::PatchDescriptor::Type type)
//{
//	bits.patch_type = type;
//}

glslProgram::glsl_info::glsl_info()/* :
	vert(DEFAULT_VERTEX_SHADER),frag(DEFAULT_FRAGMENT_SHADER),geom(DEFAULT_GEOMETRY_SHADER)*/
{
}

glslProgram::glsl_info::glsl_info(const glsl_info & info) :
	vert(info.vert), frag(info.vert), geom(info.vert), tcs(info.vert), tes(info.vert)
{
}

glslProgram::glsl_info::glsl_info(const std::string & vert, const std::string & frag, const std::string & geom, const std::string & tcs, const std::string & tes) :
	vert(vert),frag(frag),geom(geom),tcs(tcs),tes(tes)
{
}

const std::string glslProgram::glsl_info::str() const
{
	std::stringstream ss;
	ss << vert << frag << geom << tcs << tes;
	return ss.str();
}

void glslProgram::glsl_info::CreateUniformBuffer(const std::string & name, int size)
{
	auto it = buffers.find(name);
	if (it != buffers.end());
	else
		buffers.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(name),
			std::forward_as_tuple(new tv::glShaderUniformBuffer(size)));
}

const tv::glShaderUniformBuffer * glslProgram::glsl_info::GetUniformBuffer(const std::string & name)
{
	try {
		return buffers.at(name).get();
	}
	catch (std::out_of_range& e) {
		std::cout << name << " ÇÕë∂ç›ÇµÇ‹ÇπÇÒ" << std::endl;
		throw;
	}
}

std::map<std::string, std::unique_ptr<tv::glShaderUniformBuffer>> glslProgram::glsl_info::buffers;
