#pragma once

#include <vector>

#include <GL\glew.h>
#include <glm\glm.hpp>
#include <nlohmann/json.hpp>

namespace tv {
class material {
public:
	material() = delete;
	material(const nlohmann::json& j);
	material(const material& mat)= delete;
	material(material&& mat) noexcept;
	~material();
	material& operator=(const material& mat)= delete;
	material& operator=(material&& mat) noexcept;

	struct Phong {
		glm::vec3 ka;
		glm::vec3 kd;
		glm::vec3 ks;
		float     shine;
		Phong(const nlohmann::json& j);
	};

	GLuint GetTexture();
	int    GetIndex(const std::string& name);
	int    GetElementSize();

private:
	std::vector<std::string> names;
	std::vector<Phong>       material_data;
	GLuint                   texture;
};
} // namespace tv
