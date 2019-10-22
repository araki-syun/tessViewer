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

	const glslProgram& Get(const glslProgram::glsl_info& glsl);
	const glslProgram& Get(const glslProgram::glsl_info& glsl,
						   const osd_info&               osd);

	using shader_iterator =
		std::unordered_map<std::string,
						   std::unique_ptr<glslProgram>>::const_iterator;

private:
	shader_iterator add(const glslProgram::glsl_info& glsl);
	shader_iterator add(const glslProgram::glsl_info& glsl,
						const osd_info&               osd);

	std::unordered_map<std::string, std::unique_ptr<glslProgram>> shader_list;
};
} // namespace tv
