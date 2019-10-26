#include "material.h"

namespace tv {
material::material(const nlohmann::json& j) : texture(0) {
	for (auto& mat : j) {
		auto& element = mat["element"];
		material_data.emplace_back(element);
		names.emplace_back(mat["material_name"].get<std::string>());
	}
	//for (picojson::value& value : materials) {
	//	picojson::object& mat     = value.get<picojson::object>();
	//	picojson::object& element = mat["elements"].get<picojson::object>();
	//	material_data.emplace_back(Phong(element));
	//	names.emplace_back(mat["material_name"].get<std::string>());
	//}
	GLuint buffer;
	glCreateBuffers(1, &buffer);
	glNamedBufferData(buffer, material_data.size() * sizeof(Phong),
					  material_data.data(), GL_STATIC_DRAW);
	glCreateTextures(GL_TEXTURE_BUFFER, 1, &texture);
	glTextureBuffer(texture, GL_R32F, buffer);
	glDeleteBuffers(1, &buffer);
}
material::~material() {
	if (texture != 0U) {
		glDeleteTextures(1, &texture);
	}
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
	//picojson::array Ka = element["ka"].get<picojson::array>();
	//picojson::array Kd = element["kd"].get<picojson::array>();
	//picojson::array Ks = element["ks"].get<picojson::array>();

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
	//for (int i = 0; i < (int)Ka.size(); ++i)
	//	ka[i] = (float)Ka[i].get<double>();
	//for (int i = 0; i < (int)Kd.size(); ++i)
	//	kd[i] = (float)Kd[i].get<double>();
	//for (int i = 0; i < (int)Ks.size(); ++i)
	//	ks[i] = (float)Ks[i].get<double>();
	//shine = (float)element["shine"].get<double>();
}
} // namespace tv
