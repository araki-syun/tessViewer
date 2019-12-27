#include "ShaderManager.h"

#include "log.h"

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
ShaderManager::ShaderManager() = default;

ShaderManager::~ShaderManager() = default;

ShaderManager::shader_iterator
ShaderManager::_add(const GlslProgram::GlslInfo& glsl) {
	auto it = _shader_list.emplace(
		std::piecewise_construct, std::forward_as_tuple(glsl.Str()),
		std::forward_as_tuple(new GlslProgram(glsl)));
	if (it.second) {
		Logger::Log<LogLevel::Trace>(InfoType::Graphics, "ShaderManager Add");
		return it.first;
	}
	throw GraphicsError(LogLevel::Error, "Shader Insert ERROR");
}

ShaderManager::shader_iterator
ShaderManager::_add(const GlslProgram::GlslInfo& glsl, const OsdInfo& osd) {
	auto it = _shader_list.emplace(
		std::piecewise_construct, std::forward_as_tuple(glsl.Str() + osd.Str()),
		std::forward_as_tuple(new GlslProgram(glsl, osd)));
	if (it.second) {
		Logger::Log<LogLevel::Trace>(InfoType::Graphics, "ShaderManager Add");
		return it.first;
	}
	throw GraphicsError(LogLevel::Error, "Shader Insert ERROR");
}

const GlslProgram& ShaderManager::Get(const GlslProgram::GlslInfo& glsl) {
	auto it = _shader_list.find(glsl.Str());
	if (it != _shader_list.cend()) {
		return *(it->second.get());
	}
	auto it2 = _add(glsl);
	return *(it2->second.get());
}

const GlslProgram& ShaderManager::Get(const GlslProgram::GlslInfo& glsl,
									  const OsdInfo&               osd) {
	auto it = _shader_list.find(glsl.Str() + osd.Str());
	if (it != _shader_list.cend()) {
		return *(it->second.get());
	}
	auto it2 = _add(glsl, osd);
	return *(it2->second.get());
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
