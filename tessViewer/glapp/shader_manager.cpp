#include "shader_manager.h"

namespace glapp {
std::unordered_map<std::string, std::unique_ptr<Shader>>
	ShaderManager::shader_list;

Shader* ShaderManager::GetPointer(const std::string& name) {
	// std::unordered_map<std::string, std::unique_ptr<Shader> >::_Pairib p;
	auto p = shader_list.insert(std::make_pair(name, nullptr));

	if (p.second) {
		std::unique_ptr<Shader> g(new Shader(name));
		if (g->GetSource().empty()) {
			shader_list.erase(name);
			return nullptr;
		}
		p.first->second = std::move(g);
	}

	return p.first->second.get();
}
} // namespace glapp
