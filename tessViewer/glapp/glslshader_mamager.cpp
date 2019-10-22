#include "glslshader_manager.h"

namespace glapp {
std::unordered_map<std::string, std::unique_ptr<glslshader>>
	glslshader_manager::shader_list;

glslshader* glslshader_manager::GetPointer(const std::string& name) {
	// std::unordered_map<std::string, std::unique_ptr<glslshader> >::_Pairib p;
	auto p = shader_list.insert(std::make_pair(name, nullptr));

	if (p.second) {
		std::unique_ptr<glslshader> g(new glslshader(name));
		if (g->GetSource().empty()) {
			shader_list.erase(name);
			return nullptr;
		}
		p.first->second = std::move(g);
	}

	return p.first->second.get();
}
} // namespace glapp
