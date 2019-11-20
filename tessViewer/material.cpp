#include "material.h"

namespace tv {
Material::Material(const nlohmann::json& j) : _texture(0) {
	for (auto& mat : j) {
		auto& element = mat["element"];
		_material_data.emplace_back(element);
		_names.emplace_back(mat["material_name"].get<std::string>());
	}

	GLuint buffer;
	glCreateBuffers(1, &buffer);
	glNamedBufferData(buffer, _material_data.size() * sizeof(Phong),
					  _material_data.data(), GL_STATIC_DRAW);
	glCreateTextures(GL_TEXTURE_BUFFER, 1, &_texture);
	glTextureBuffer(_texture, GL_R32F, buffer);
	glDeleteBuffers(1, &buffer);
}
Material::Material(Material&& mat) noexcept
	: _names(std::move(mat._names))
	, _material_data(std::move(mat._material_data))
	, _texture(std::move(mat._texture)) {}
Material::~Material() {
	if (_texture != 0U) {
		glDeleteTextures(1, &_texture);
	}
}
Material& Material::operator=(Material&& mat) noexcept {
	if (this != &mat) {
		_names         = std::move(mat._names);
		_material_data = std::move(mat._material_data);
		if (_texture != 0U) {
			glDeleteTextures(1, &_texture);
		}
		_texture = std::move(mat._texture);
	}

	return *this;
}

GLuint Material::GetTexture() { return _texture; }

int Material::GetIndex(const std::string& name) {
	for (int i = 0; i < (int)_names.size(); ++i) {
		if (name == _names[i]) {
			return i;
		}
	}
	return -1;
}

int Material::GetElementSize() { return sizeof(Phong) / sizeof(float); }

Material::Phong::Phong(const nlohmann::json& j) {
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
