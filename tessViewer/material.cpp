#include "material.h"

namespace tv {
material::material(const nlohmann::json& j) : texture(0) {
	for (auto& mat : j) {
		auto& element = mat["element"];
		material_data.emplace_back(element);
		names.emplace_back(mat["material_name"].get<std::string>());
	}

	GLuint buffer;
	glCreateBuffers(1, &buffer);
	glNamedBufferData(buffer, material_data.size() * sizeof(Phong),
					  material_data.data(), GL_STATIC_DRAW);
	glCreateTextures(GL_TEXTURE_BUFFER, 1, &texture);
	glTextureBuffer(texture, GL_R32F, buffer);
	glDeleteBuffers(1, &buffer);
}
material::material(material&& mat) noexcept
	: names(std::move(mat.names))
	, material_data(std::move(mat.material_data))
	, texture(std::move(mat.texture)) {}
material::~material() {
	if (texture != 0U) {
		glDeleteTextures(1, &texture);
	}
}
material& material::operator=(material&& mat) noexcept {
	if (this != &mat) {
		names         = std::move(mat.names);
		material_data = std::move(mat.material_data);
		if (texture != 0U) {
			glDeleteTextures(1, &texture);
		}
		texture = std::move(mat.texture);
	}

	return *this;
}

GLuint material::GetTexture() { return texture; }

int material::GetIndex(const std::string& name) {
	for (int i = 0; i < (int)names.size(); ++i) {
		if (name == names[i]) {
			return i;
		}
	}
	return -1;
}

int material::GetElementSize() { return sizeof(Phong) / sizeof(float); }

material::Phong::Phong(const nlohmann::json& j) {
	auto& a = j["Ka"];
	auto& d = j["Kd"];
	auto& s = j["Ks"];

	if (a.is_array()) {
		for (int i = 0; i < 3; ++i) {
			ka[i] = a[i].get<double>();
		}
	}
	if (d.is_array()) {
		for (int i = 0; i < 3; ++i) {
			kd[i] = d[i].get<double>();
		}
	}
	if (s.is_array()) {
		for (int i = 0; i < 3; ++i) {
			ks[i] = s[i].get<double>();
		}
	}
	shine = j["shine"].get<double>();
}
} // namespace tv
