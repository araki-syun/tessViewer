#include "model.h"

#include <iostream>
#include <memory>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <opensubdiv\far\topologyRefiner.h>
#include <opensubdiv\far\stencilTable.h>
#include <opensubdiv\far\primvarRefiner.h>
#include <opensubdiv\osd\glVertexBuffer.h>

#include "glapp\glapp.hpp"
#include "define.h"
#include "glapp\config.h"
#include "log.h"

//OpenSubdiv::Far::TopologyRefiner* createRefiner(const picojson::object& obj);

namespace tv {
Model::Model(const tv::Model& m)
	: _name(m._name)
	, _vertex(m._vertex)
	, _normal(m._normal)
	, _vao(0)
	, _mesh(nullptr)
	, _program(m._program)
	, _shader_osd_info(m._shader_osd_info)
	, _model_matrix(m._model_matrix)
	, _texcoord_fvar_texture(0)
	, _material_index_texture(0) {}
Model::Model(const nlohmann::json& obj, std::shared_ptr<tv::Material> mat)
	: _vao(0), _material(mat) {
	Reload(obj);
}
Model::~Model() {
	if (_vao != 0u) {
		glDeleteVertexArrays(1, &_vao);
	}
	if (_texcoord_fvar_texture != 0u) {
		glDeleteTextures(1, &_texcoord_fvar_texture);
	}
	if (_material_index_texture != 0u) {
		glDeleteTextures(1, &_material_index_texture);
	}
	Logger::Log<LogLevel::Debug>(InfoType::Graphics, "Model Destructed");
}

void Model::Reload(const nlohmann::json& obj) {
	using namespace OpenSubdiv;

	Logger::Log<LogLevel::Trace>(InfoType::Graphics, "Model Loading Start");
	Far::TopologyDescriptor desc;

	auto& vertex_positions  = obj["vertex_positions"];
	auto& vertex_normals    = obj["vertex_normals"];
	auto& vertex_uvs        = obj["vertex_uvs"];
	auto& faces             = obj["faces"];
	auto& vert_per_face     = obj["vert_per_face"];
	auto& crease_weights    = obj["crease_weights"];
	auto& crease_points     = obj["crease_points"];
	auto& crease_edges      = obj["crease_edges"];
	auto& vertex_groups     = obj["vertex_groups"];
	auto& face_groups       = obj["face_groups"];
	auto& material_per_face = obj["material_per_face"];
	auto& location          = obj["location"];
	auto& rotation          = obj["rotation"];
	auto& scale             = obj["scale"];

	_vertex.reserve(vertex_positions.size());
	_normal.reserve(vertex_normals.size());

	std::vector<float>     v;
	std::vector<float>     n;
	std::vector<float>     u;
	std::vector<int>       f;
	std::vector<int>       vf;
	std::vector<float>     w;
	std::vector<int>       p;
	std::vector<int>       e;
	std::vector<int>       mpf;
	std::vector<int>       uv_indices;
	std::vector<glm::vec2> uv_pool;

	std::map<std::string, std::vector<int>>          vg;
	std::map<std::string, std::vector<int>>          fg;
	std::vector<std::pair<std::string, std::string>> mf;

	v.reserve(vertex_positions.size());
	n.reserve(vertex_normals.size());
	u.reserve(vertex_uvs.size());
	f.reserve(faces.size());
	vf.reserve(vert_per_face.size());
	w.reserve(crease_weights.size());
	p.reserve(crease_points.size());
	e.reserve(crease_edges.size());
	mpf.reserve(vert_per_face.size());
	mf.reserve(material_per_face.size());
	uv_indices.reserve(faces.size());
	uv_pool.reserve(faces.size());

	for (auto& val : vertex_positions) {
		v.emplace_back(val.get<double>());
	}
	for (auto& val : vertex_normals) {
		n.emplace_back(val.get<double>());
	}
	for (auto& val : vertex_uvs) {
		u.emplace_back(val.get<double>());
	}
	for (auto& val : faces) {
		f.emplace_back(val.get<double>());
	}
	for (auto& val : vert_per_face) {
		vf.emplace_back(val.get<double>());
	}
	for (auto& val : crease_weights) {
		w.emplace_back(val.get<double>() * 10.0);
	}
	for (auto& val : crease_points) {
		p.emplace_back(val.get<double>());
	}
	for (auto& val : crease_edges) {
		e.emplace_back(val.get<double>());
	}
	for (auto& val : vertex_groups) {
		std::vector<int>& indices  = vg[val["group_name"].get<std::string>()];
		auto&             vertices = val["vertices"];
		indices.reserve(vertices.size());
		for (auto& index : vertices) {
			indices.emplace_back(index.get<double>());
		}
	}
	for (auto& val : face_groups) {
		std::vector<int>& indices = fg[val["group_name"].get<std::string>()];
		auto&             faces   = val["faces"];

		indices.reserve(faces.size());
		for (auto& index : faces) {
			indices.emplace_back(index.get<double>());
		}
	}
	for (auto& val : material_per_face) {
		mf.emplace_back(
			std::make_pair(val["material_name"].get<std::string>(),
						   val["face_group_name"].get<std::string>()));
		val["face_group_name"].get<std::string>();
	}
	for (std::pair<std::string, std::string>& p : mf) {
		auto it     = fg.find(p.second);
		int  mat_id = _material->GetIndex(p.first);
		for (int id : it->second) {
			mpf[id] = mat_id;
		}
	}
	for (int i = 0, id = 0; i < u.size() / 2; ++i) {
		glm::vec2 uv = glm::make_vec2(&u[i * 2]);
		auto      it = std::find_if(uv_pool.cbegin(), uv_pool.cend(),
                               [&](const glm::vec2& v) {
                                   return glm::length(uv - v) < 1.f / 4096.f;
                               });
		if (it != uv_pool.cend()) {
			uv_indices.emplace_back(std::distance(uv_pool.cbegin(), it));
		} else {
			uv_pool.emplace_back(uv);
			uv_indices.emplace_back(id++);
		}
	}

	//create descriptor
	Far::TopologyDescriptor::FVarChannel fvar;
	fvar.numValues    = (int)uv_pool.size();
	fvar.valueIndices = uv_indices.data();

	desc.numVertices        = (int)v.size() / 3;
	desc.numFaces           = (int)vf.size();
	desc.numVertsPerFace    = vf.data();
	desc.vertIndicesPerFace = f.data();
	if (!e.empty()) {
		desc.numCreases             = (int)e.size() / 2;
		desc.creaseWeights          = w.data();
		desc.creaseVertexIndexPairs = e.data();
	}
	if (!p.empty()) {
		desc.numCorners          = (int)p.size();
		desc.cornerWeights       = &w[e.size() / 2];
		desc.cornerVertexIndices = p.data();
	}
	desc.numFVarChannels = 1;
	desc.fvarChannels    = &fvar;

	Logger::Log<LogLevel::Trace>(InfoType::Graphics, "Model Loading Finish");
	Logger::Log<LogLevel::Trace>(InfoType::Graphics, "OSD Model Create Start");

	// create refiner
	Sdc::Options opt;
	opt.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_AND_CORNER);

	Far::TopologyRefiner* refiner(
		Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Create(
			desc, Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Options(
					  Sdc::SchemeType::SCHEME_CATMARK, opt)));

	int num_vertex_elements = 3; // x, y, z
	//int numVaryingElements = 3; // normal

	_shader_osd_info.elem.bits.fvar_width          = 2;
	_shader_osd_info.elem.bits.is_adaptive         = 1;
	_shader_osd_info.elem.bits.patch_cull          = 1;
	_shader_osd_info.elem.bits.screen_space_tess   = 1;
	_shader_osd_info.elem.bits.single_crease_patch = 1;
	_shader_osd_info.elem.bits.fractional          = 1;
	_shader_osd_info.elem.SetPatchType(default_patch_type);

	Osd::MeshBitset bits;
	bits.set(Osd::MeshAdaptive,
			 _shader_osd_info.elem.bits.is_adaptive == 1u); // set adaptive
	bits.set(Osd::MeshEndCapBSplineBasis,
			 _shader_osd_info.elem.GetPatchType() ==
				 Far::PatchDescriptor::Type::
					 REGULAR); // use b-spline basis patch for endcap.
	bits.set(Osd::MeshEndCapGregoryBasis,
			 _shader_osd_info.elem.GetPatchType() ==
				 Far::PatchDescriptor::Type::GREGORY_BASIS);
	bits.set(Osd::MeshUseSingleCreasePatch,
			 _shader_osd_info.elem.bits.single_crease_patch == 1u);
	bits.set(Osd::MeshFVarData, _shader_osd_info.elem.bits.fvar_width != 0u);

	_mesh =
		std::make_unique<Osd::Mesh<Osd::GLVertexBuffer, Osd::GLStencilTableSSBO,
								   Osd::GLComputeEvaluator, Osd::GLPatchTable>>(
			refiner, num_vertex_elements, 0,
			glm::clamp(default_patch, 0, max_patch), bits, &evaluator);

	// 面ごとに変化するデータを設定する
	GLuint matbuffer;
	glCreateBuffers(1, &matbuffer);
	glNamedBufferData(matbuffer, mpf.size() * sizeof(int), mpf.data(),
					  GL_STATIC_DRAW);
	glCreateTextures(GL_TEXTURE_BUFFER, 1, &_material_index_texture);
	glTextureBuffer(_material_index_texture, GL_R32I, matbuffer);
	glDeleteBuffers(1, &matbuffer);

	// Face-Varying
	Far::PrimvarRefiner    primvar(*refiner);
	std::vector<glm::vec2> fvaruv(uv_pool);
	fvaruv.resize(refiner->GetNumFVarValuesTotal(0));
	auto* src = reinterpret_cast<FVarUV*>(fvaruv.data());
	for (int level = 1; level <= refiner->GetMaxLevel(); ++level) {
		FVarUV* dst = src + refiner->GetLevel(level - 1).GetNumFVarValues(0);
		primvar.InterpolateFaceVarying(level, src, dst, 0);
		src = dst;
	}
	Far::ConstIndexArray indices = _mesh->GetFarPatchTable()->GetFVarValues(0);
	std::vector<glm::vec2> uv_data;
	uv_data.reserve(indices.size()); // *FVarUV::GetWidth());
	for (int indice : indices) {
		//int index = indices[i];// *FVarUV::GetWidth();
		//for (int j = 0; j < FVarUV::GetWidth(); ++j) {
		uv_data.emplace_back(fvaruv[indice]); // [j]);
											  //}
	}
	GLuint fvar_buffer;
	glCreateBuffers(1, &fvar_buffer);
	glNamedBufferData(fvar_buffer, uv_data.size() * sizeof(glm::vec2),
					  uv_data.data(), GL_STATIC_DRAW);
	glCreateTextures(GL_TEXTURE_BUFFER, 1, &_texcoord_fvar_texture);
	glTextureBuffer(_texcoord_fvar_texture, GL_R32F, fvar_buffer);
	glDeleteBuffers(1, &fvar_buffer);

	Update(v.data(), 0, (int)v.size() / 3);

	Logger::Log<LogLevel::Trace>(InfoType::Graphics, "OSD Model Create Finish");

	if (_vao != 0u) {
		glDeleteVertexArrays(1, &_vao);
	}
	glCreateVertexArrays(1, &_vao);

	// インデックスバッファ
	glVertexArrayElementBuffer(_vao,
							   _mesh->GetPatchTable()->GetPatchIndexBuffer());
	// 頂点バッファ
	const GlslProgram& program =
		shader_manage->Get(default_glsl_info, _shader_osd_info);
	GLuint pos_index = program.GetAttrib("position")->GetIndex();
	GLuint vertbuf   = _mesh->BindVertexBuffer();
	//varybuf = mesh->BindVaryingBuffer();

	// Vertex Position
	glVertexArrayVertexBuffer(_vao, 0, vertbuf, 0,
							  sizeof(float) * num_vertex_elements);
	glVertexArrayAttribFormat(_vao, pos_index, num_vertex_elements, GL_FLOAT,
							  GL_FALSE, 0);
	glVertexArrayAttribBinding(_vao, pos_index, 0);
	glEnableVertexArrayAttrib(_vao, pos_index);

	// パッチパラメータバッファ
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_BUFFER, mesh->GetPatchTable()->GetPatchParamTextureBuffer());
	glBindTextureUnit(0, _mesh->GetPatchTable()->GetPatchParamTextureBuffer());
	//glBindTextureUnit(1, mesh->GetPatchTable()->GetPatchIndexTextureBuffer());
	
	Logger::Log<LogLevel::Trace>(InfoType::Graphics, "Model Data Upload to VRAM");

	glm::mat4 trans = glm::translate(glm::vec3(location[0].get<double>(),
											   location[1].get<double>(),
											   location[2].get<double>()));

	glm::mat4 rot = glm::eulerAngleXYZ(((float)rotation[0].get<double>()),
									   ((float)rotation[1].get<double>()),
									   ((float)rotation[2].get<double>()));

	glm::mat4 sc =
		glm::scale(glm::vec3(scale[0].get<double>(), scale[1].get<double>(),
							 scale[2].get<double>()));

	_model_matrix = trans * rot * sc;

	Logger::Log<LogLevel::Debug>(InfoType::Graphics, "Model Created");
}

void Model::Update(const float* v1,
				   int          start1,
				   int          size1,
				   const float* v2,
				   int          start2,
				   int          size2) {
	_mesh->UpdateVertexBuffer(v1, start1, size1);
	if (v2 != nullptr) {
		_mesh->UpdateVaryingBuffer(v2, start2, size2);
	}
	_mesh->Refine();
	_mesh->Synchronize();
}

void Model::Draw() {
	using namespace OpenSubdiv;

	if (_shader_osd_info.elem.bits.patch_cull == 0u) {
		glDisable(GL_CULL_FACE);
	}
	_mesh->BindVertexBuffer();
	if (_mesh->GetPatchTable()->GetPatchParamTextureBuffer() != 0u) {
		glBindTextureUnit(0,
						  _mesh->GetPatchTable()->GetPatchParamTextureBuffer());
	}
	//if (mesh->GetPatchTable()->GetPatchIndexTextureBuffer())
	//	glBindTextureUnit(1, mesh->GetPatchTable()->GetPatchIndexTextureBuffer());
	if (_texcoord_fvar_texture != 0u) {
		glBindTextureUnit(1, _texcoord_fvar_texture);
	}
	if (_material_index_texture != 0u) {
		glBindTextureUnit(2, _material_index_texture);
	}
	if (_material->GetTexture() != 0u) {
		glBindTextureUnit(3, _material->GetTexture());
	}
	if (default_texture != 0u) {
		glBindTextureUnit(4, default_texture);
	}
	glBindVertexArray(_vao);
	for (const auto& patch : _mesh->GetPatchTable()->GetPatchArrays()) {
		const auto& desc = patch.GetDescriptor();

		int num_verts_per_patch =
			desc.GetNumControlVertices(); // 16 for B-spline patches
		OsdInfo info(_shader_osd_info);
		info.elem.SetPatchType(desc.GetType());
		const GlslProgram& program =
			shader_manage->Get(default_glsl_info, info);
		GLuint prog = program.GetProgram();
		glUseProgram(prog);
		glProgramUniform1i(
			prog, program.GetUniform("OsdPatchParamBuffer")->GetIndex(), 0);
		glProgramUniform1i(
			prog, program.GetUniform("OsdFVarDataBuffer")->GetIndex(), 1);
		glProgramUniform1i(
			prog, program.GetUniform("MaterialIndexBuffer")->GetIndex(), 2);
		glProgramUniform1i(
			prog, program.GetUniform("MaterialDataBuffer")->GetIndex(), 3);
		glProgramUniform1i(prog, program.GetUniform("diffuse_map")->GetIndex(),
						   4);
		glProgramUniform1i(prog,
						   program.GetUniform("PrimitiveIdBase")->GetIndex(),
						   patch.GetPrimitiveIdBase());
		glProgramUniformMatrix4fv(prog, program.GetUniform("Model")->GetIndex(),
								  1, GL_FALSE, &_model_matrix[0][0]);
		glPatchParameteri(GL_PATCH_VERTICES, num_verts_per_patch);
		glDrawElements(GL_PATCHES, patch.GetNumPatches() * num_verts_per_patch,
					   GL_UNSIGNED_INT,
					   (void*)(patch.GetIndexBase() * sizeof(unsigned int)));
		++draw_call;
	}
	glBindVertexArray(0);
	glUseProgram(0);

	if (_shader_osd_info.elem.bits.patch_cull == 0u) {
		glEnable(GL_CULL_FACE);
	}
}
const GlslProgram* Model::GetProgram() const { return _program.get(); }
void               Model::SetProgram(std::shared_ptr<GlslProgram> prog) {
    if (prog) {
        _program = prog;
    }
}

int                                    Model::draw_call     = 0;
int                                    Model::max_patch     = 6;
int                                    Model::default_patch = 2;
OpenSubdiv::Far::PatchDescriptor::Type Model::default_patch_type;
GLuint                                 Model::default_texture = 0;
GlslProgram::GlslInfo                  Model::default_glsl_info;
ShaderManager*                         Model::shader_manage;
OpenSubdiv::Osd::EvaluatorCacheT<OpenSubdiv::Osd::GLComputeEvaluator>
	Model::evaluator;
} // namespace tv
