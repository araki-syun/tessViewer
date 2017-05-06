#include "material.h"


namespace tv {
	material::material(picojson::array& materials) :
		texture(0)
	{
		for (picojson::value& value : materials) {
			picojson::object& mat = value.get<picojson::object>();
			picojson::object& element = mat["elements"].get<picojson::object>();
			material_data.emplace_back(Phong(element));
			names.emplace_back(mat["material_name"].get<std::string>());
		}
		GLuint buffer;
		glCreateBuffers(1, &buffer);
		glNamedBufferData(buffer, material_data.size() * sizeof(Phong), material_data.data(), GL_STATIC_DRAW);
		glCreateTextures(GL_TEXTURE_BUFFER, 1, &texture);
		glTextureBuffer(texture, GL_R32F, buffer);
		glDeleteBuffers(1, &buffer);
	}
	material::~material()
	{
		if (texture)
			glDeleteTextures(1, &texture);
	}

	GLuint material::GetTexture()
	{
		return texture;
	}

	int material::GetIndex(const std::string & name)
	{
		for (int i = 0; i < (int)names.size(); ++i)
			if (name == names[i])
				return i;
	}

	int material::GetElementSize()
	{
		return sizeof(Phong) / sizeof(float);
	}


	material::Phong::Phong(picojson::object & element)
	{
		picojson::array Ka = element["ka"].get<picojson::array>();
		picojson::array Kd = element["kd"].get<picojson::array>();
		picojson::array Ks = element["ks"].get<picojson::array>();

		for (int i = 0; i < (int)Ka.size(); ++i)
			ka[i] = (float)Ka[i].get<double>();
		for (int i = 0; i < (int)Kd.size(); ++i)
			kd[i] = (float)Kd[i].get<double>();
		for (int i = 0; i < (int)Ks.size(); ++i)
			ks[i] = (float)Ks[i].get<double>();
		shine = (float)element["shine"].get<double>();
	}
}
