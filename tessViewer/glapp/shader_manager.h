#pragma once

#include <unordered_map>
#include <memory>

#include <boost\noncopyable.hpp>

#include "shader_object.h"

namespace glapp {
class ShaderManager : boost::noncopyable {
	ShaderManager();

public:
	//static glslshader_manager* GetInstance();

	static Shader* GetPointer(const std::string& name);

private:
	//static std::unique_ptr<glslshader_manager> p;

	static std::unordered_map<std::string, std::unique_ptr<Shader>> shader_list;
};
} // namespace glapp
