#pragma once

#include <unordered_map>
#include <memory>
#include <stdexcept>

#include "osd_info.h"
#include "glslProgram.h"

//class glShaderUniformBuffer {
//public:
//	glShaderUniformBuffer(/*GLuint program,*/ int buf_size, const char* name);
//	~glShaderUniformBuffer();
//
//	void Update(const void* val) const;
//	void Set(/*GLuint program,*/ const char* name);
//	GLuint GetBuffer();
//	GLuint GetBinding();
//
//private:
//	static GLuint _bindindex;
//	GLuint _binding;
//	GLuint _buffer;
//	int _bufSize;
//};
namespace tv {
class ShaderManager {
public:
	ShaderManager();
	~ShaderManager();

	const GlslProgram& Get(const GlslProgram::GlslInfo& glsl);
	const GlslProgram& Get(const GlslProgram::GlslInfo& glsl,
						   const OsdInfo&               osd);

	using shader_iterator =
		std::unordered_map<std::string,
						   std::unique_ptr<GlslProgram>>::const_iterator;

private:
	shader_iterator _add(const GlslProgram::GlslInfo& glsl);
	shader_iterator _add(const GlslProgram::GlslInfo& glsl, const OsdInfo& osd);

	std::unordered_map<std::string, std::unique_ptr<GlslProgram>> _shader_list;
};
} // namespace tv
