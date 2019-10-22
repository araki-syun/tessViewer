#include "ShaderManager.h"

//glShaderUniformBuffer::glShaderUniformBuffer(/*GLuint program,*/ int buf_size, const char * name) :
//	_binding(_bindindex++),_buffer(0), _bufSize(buf_size)
//{
//	this->Set(/*program,*/ name);
//}
//glShaderUniformBuffer::~glShaderUniformBuffer()
//{
//	if (_buffer)
//		glDeleteBuffers(1, &_buffer);
//	--_bindindex;
//}
//
//void glShaderUniformBuffer::Update(const void * val) const
//{
//	glNamedBufferSubData(_buffer, 0, _bufSize, val);
//
//	glBindBufferBase(GL_UNIFORM_BUFFER, _binding, _buffer);
//}
//void glShaderUniformBuffer::Set(/*GLuint program,*/ const char * name)
//{
//	if (_buffer)
//		glDeleteBuffers(1, &_buffer);
//	glCreateBuffers(1, &_buffer);
//	glNamedBufferData(_buffer, _bufSize, nullptr, GL_DYNAMIC_DRAW);
//}
//GLuint glShaderUniformBuffer::GetBuffer()
//{
//	return _buffer;
//}
//GLuint glShaderUniformBuffer::GetBinding()
//{
//	return _binding;
//}
//
//GLuint glShaderUniformBuffer::_bindindex = 0;

namespace tv {
ShaderManager::ShaderManager() {}

ShaderManager::~ShaderManager() {}

ShaderManager::shader_iterator
ShaderManager::add(const glslProgram::glsl_info& glsl) {
	auto it = shader_list.emplace(std::piecewise_construct,
								  std::forward_as_tuple(glsl.str()),
								  std::forward_as_tuple(new glslProgram(glsl)));
	if (it.second) {
		return it.first;
	} else
		throw std::runtime_error("Shader Insert ERROR");
}

ShaderManager::shader_iterator
ShaderManager::add(const glslProgram::glsl_info& glsl, const osd_info& osd) {
	auto it = shader_list.emplace(
		std::piecewise_construct, std::forward_as_tuple(glsl.str() + osd.str()),
		std::forward_as_tuple(new glslProgram(glsl, osd)));
	if (it.second) {
		return it.first;
	} else
		throw std::runtime_error("Shader Insert ERROR");
}

const glslProgram& ShaderManager::Get(const glslProgram::glsl_info& glsl) {
	auto it = shader_list.find(glsl.str());
	if (it != shader_list.cend())
		return *(it->second.get());
	else {
		auto it2 = add(glsl);
		return *(it2->second.get());
	}
}

const glslProgram& ShaderManager::Get(const glslProgram::glsl_info& glsl,
									  const osd_info&               osd) {
	auto it = shader_list.find(glsl.str() + osd.str());
	if (it != shader_list.cend())
		return *(it->second.get());
	else {
		auto it2 = add(glsl, osd);
		return *(it2->second.get());
	}
}

//const glShaderUniformBuffer * ShaderManager::GetUniformBuffer(const std::string & name)
//{
//	auto it = uniformBuffer_map.find(name);
//	if (it != uniformBuffer_map.cend())
//		return it->second.get();
//	return nullptr;
//}

//void ShaderManager::GenUniformBuffer(const std::string & name, int buf_size)
//{
//	uniformBuffer_map[name].reset(new glShaderUniformBuffer(buf_size, name.c_str()));
//}
} // namespace tv
