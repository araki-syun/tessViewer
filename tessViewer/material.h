#pragma once

#include <vector>

#include <GL\glew.h>
#include <glm\glm.hpp>
#include <picojson.h>

namespace tv {
class material {
public:
	material() = delete;
	material(picojson::array& materials);
	~material();

	struct Phong {
		glm::vec3 ka;
		glm::vec3 kd;
		glm::vec3 ks;
		float     shine;
		Phong(picojson::object& element);
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
