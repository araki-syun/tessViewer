#pragma once

#include <vector>

#include <GL\glew.h>
#include <glm\glm.hpp>
#include <nlohmann/json.hpp>

namespace tv {
class Material {
public:
	Material() = delete;
	Material(const nlohmann::json& j);
	Material(const Material& mat) = delete;
	Material(Material&& mat) noexcept;
	~Material();
	Material& operator=(const Material& mat) = delete;
	Material& operator                       =(Material&& mat) noexcept;

	struct Phong {
		glm::vec3 ka{};
		glm::vec3 kd{};
		glm::vec3 ks{};
		float     shine{};
		Phong(const nlohmann::json& j);
	};

	GLuint GetTexture();
	int    GetIndex(const std::string& name);
	int    GetElementSize();

private:
	std::vector<std::string> _names;
	std::vector<Phong>       _material_data;
	GLuint                   _texture;
};
} // namespace tv
