#pragma once

#include <unordered_map>
#include <memory>

#include <boost\noncopyable.hpp>

#include "glslshader.h"

namespace glapp {
class glslshader_manager : boost::noncopyable {
	glslshader_manager();

public:
	//static glslshader_manager* GetInstance();

	static glslshader* GetPointer(const std::string& name);

private:
	//static std::unique_ptr<glslshader_manager> p;

	static std::unordered_map<std::string, std::unique_ptr<glslshader>>
		shader_list;
};
} // namespace glapp
