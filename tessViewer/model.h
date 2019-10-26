#pragma once

#include <vector>
#include <string>

#include <boost\noncopyable.hpp>

#include <GL\glew.h>
#include <glm\glm.hpp>

#include <opensubdiv\osd\opengl.h>
#include <opensubdiv\far\topologyDescriptor.h>
#include <opensubdiv\osd\glMesh.h>
#include <opensubdiv\osd\glComputeEvaluator.h>

#include <nlohmann/json.hpp>

#include "ShaderManager.h"
#include "material.h"

namespace tv {
class model : public boost::noncopyable {
public:
	model();
	model(const tv::model& m);
	model(const nlohmann::json& obj, std::shared_ptr<tv::material> mat);
	~model();

	void               Reload(const nlohmann::json& obj);
	void               Update(const float* v1,
							  int          start1,
							  int          size1,
							  const float* v2     = nullptr,
							  int          start2 = 0,
							  int          size2  = 0);
	void               Draw();
	const glslProgram* GetProgram() const;
	void               SetProgram(std::shared_ptr<glslProgram> prog);

	static int                                    draw_call;
	static int                                    max_patch;
	static int                                    default_patch;
	static OpenSubdiv::Far::PatchDescriptor::Type default_patch_type;
	static GLuint                                 default_texture;
	static glslProgram::glsl_info                 default_glsl_info;
	static ShaderManager*                         shader_manage;

private:
	std::string        name;
	std::vector<float> vertex, normal;
	GLuint             vao; //	Vertex Attrib Object
	std::unique_ptr<OpenSubdiv::Osd::GLMeshInterface> mesh;
	std::shared_ptr<glslProgram>                      program;
	osd_info                                          shader_osd_info;
	glm::mat4                                         modelMatrix;
	GLuint                                            texcoord_fvar_texture;
	GLuint                                            material_index_texture;
	std::shared_ptr<tv::material>                     material;

	static OpenSubdiv::Osd::EvaluatorCacheT<OpenSubdiv::Osd::GLComputeEvaluator>
		evaluator;
};
//struct FVarIndex
//{
//	int id;
//	void Clear() {
//		id = 0;
//	}
//	void AddWithWeight(FVarIndex const & index, float) {
//		id = index.id;
//	}
//};
struct FVarUV {
	glm::vec2 uv;
	void      Clear() { uv = glm::vec2(0); }
	void      AddWithWeight(FVarUV const& src, float weight) {
        uv += src.uv * weight;
	}
	static int GetWidth() { return 2; }
};
} // namespace tv
