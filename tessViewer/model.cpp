#include "model.h"

#include <iostream>
#include <memory>
#include <algorithm>

#include <glm\ext.hpp>
#include <glm\gtx\quaternion.hpp>
#include <opensubdiv\far\topologyRefiner.h>
#include <opensubdiv\far\stencilTable.h>
#include <opensubdiv\far\primvarRefiner.h>
#include <opensubdiv\osd\glVertexBuffer.h>

#include "glapp\glapp.hpp"
#include "glapp\glapp_define.h"
#include "glapp\config.h"

#include "picojson.h"


//OpenSubdiv::Far::TopologyRefiner* createRefiner(const picojson::object& obj);

namespace tv {
	model::model() :
		vao(0), texcoord_fvar_texture(0),material_index_texture(0)
	{
	}
	model::model(const tv::model & m) :
		name(m.name), vertex(m.vertex), normal(m.normal), vao(0), mesh(nullptr),
		program(m.program), shader_osd_info(m.shader_osd_info), modelMatrix(m.modelMatrix),
		texcoord_fvar_texture(0), material_index_texture(0)
	{
	}
	model::model(picojson::object & obj, /*std::shared_ptr<glslProgram> p,*/ std::shared_ptr<tv::material> mat) :
		vao(0), material(mat)
	{
		//SetProgram(p);
		Reload(obj);
	}
	model::~model()
	{
		if (vao)
			glDeleteVertexArrays(1, &vao);
		if (texcoord_fvar_texture)
			glDeleteTextures(1, &texcoord_fvar_texture);
		if (material_index_texture)
			glDeleteTextures(1, &material_index_texture);
	}

	void model::Reload(picojson::object& obj)
	{
		using namespace OpenSubdiv;

		Far::TopologyDescriptor desc;

		picojson::array& vertex_positions = obj["vertex_positions"].get<picojson::array>();
		picojson::array& vertex_normals = obj["vertex_normals"].get<picojson::array>();
		picojson::array& vertex_uvs = obj["vertex_uvs"].get<picojson::array>();
		picojson::array& faces = obj["faces"].get<picojson::array>();
		picojson::array& vert_per_face = obj["vert_per_face"].get<picojson::array>();
		picojson::array& crease_weights = obj["crease_weights"].get<picojson::array>();
		picojson::array& crease_points = obj["crease_points"].get<picojson::array>();
		picojson::array& crease_edges = obj["crease_edges"].get<picojson::array>();
		picojson::array& vertex_groups = obj["vertex_groups"].get<picojson::array>();
		picojson::array& face_groups = obj["face_groups"].get<picojson::array>();
		picojson::array& material_per_face = obj["material_per_face"].get<picojson::array>();
		picojson::array& location = obj["location"].get<picojson::array>();
		picojson::array& rotation = obj["rotation"].get<picojson::array>();
		picojson::array& scale = obj["scale"].get<picojson::array>();
		vertex.reserve(vertex_positions.size());
		normal.reserve(vertex_normals.size());

		std::vector<float> v;	v.reserve(vertex_positions.size());
		std::vector<float> n;	n.reserve(vertex_normals.size());
		std::vector<float> u;	u.reserve(vertex_uvs.size());
		std::vector<int> f;		f.reserve(faces.size());
		std::vector<int> vf;	vf.reserve(vert_per_face.size());
		std::vector<float> w;	w.reserve(crease_weights.size());
		std::vector<int> p;		p.reserve(crease_points.size());
		std::vector<int> e;		e.reserve(crease_edges.size());
		std::map<std::string, std::vector<int> > vg;
		std::map<std::string, std::vector<int> > fg;
		std::vector<std::pair<std::string, std::string> > mf; mf.reserve(material_per_face.size());
		std::vector<int> mpf/*; materialPerFace.reserve*/(vert_per_face.size());
		std::vector<int> uv_indices;	uv_indices.reserve(faces.size());
		std::vector<glm::vec2> uv_pool;		uv_pool.reserve(faces.size());

		for (picojson::value& val : vertex_positions)
			v.emplace_back(val.get<double>());
		for (picojson::value& val : vertex_normals)
			n.emplace_back(val.get<double>());
		for (picojson::value& val : vertex_uvs)
			u.emplace_back(val.get<double>());
		for (picojson::value& val : faces)
			f.emplace_back(val.get<double>());
		for (picojson::value& val : vert_per_face)
			vf.emplace_back(val.get<double>());
		for (picojson::value& val : crease_weights)
			w.emplace_back(val.get<double>() * 10.0);
		for (picojson::value& val : crease_points)
			p.emplace_back(val.get<double>());
		for (picojson::value& val : crease_edges)
			e.emplace_back(val.get<double>());
		for (picojson::value& val : vertex_groups) {
			picojson::object& o = val.get<picojson::object>();
			std::vector<int>& indices = vg[o["group_name"].get<std::string>()];
			picojson::array& vertices = o["vertices"].get<picojson::array>();
			indices.reserve(vertices.size());
			for (picojson::value& index : vertices)
				indices.emplace_back(index.get<double>());
		}
		for (picojson::value& val : face_groups) {
			picojson::object& o = val.get<picojson::object>();
			std::vector<int>& indices = fg[o["group_name"].get<std::string>()];
			picojson::array& faces = o["faces"].get<picojson::array>();
			indices.reserve(faces.size());
			for (picojson::value& index : faces)
				indices.emplace_back(index.get<double>());
		}
		for (picojson::value& val : material_per_face) {
			picojson::object& o = val.get<picojson::object>();
			mf.emplace_back(std::make_pair(o["material_name"].get<std::string>(), o["face_group_name"].get<std::string>()));
			o["face_group_name"].get<std::string>();
		}
		for (std::pair<std::string, std::string>& p : mf) {
			auto it = fg.find(p.second);
			int mat_id = material->GetIndex(p.first);
			for (int id : it->second) {
				mpf[id] = mat_id;
			}
		}
		for (int i = 0,id = 0; i < u.size() / 2; ++i) {
			glm::vec2 uv = glm::make_vec2(&u[i * 2]);
			auto it = std::find_if(uv_pool.cbegin(), uv_pool.cend(), [&](const glm::vec2& v) {return glm::length(uv - v) < 1.f / 4096.f; });
			if (it != uv_pool.cend()) {
				uv_indices.emplace_back(std::distance(uv_pool.cbegin(), it));
			}
			else {
				uv_pool.emplace_back(uv);
				uv_indices.emplace_back(id++);
			}
		}

		//create descriptor
		Far::TopologyDescriptor::FVarChannel fvar;
		fvar.numValues = (int)uv_pool.size();
		fvar.valueIndices = uv_indices.data();

		desc.numVertices = (int)v.size() / 3;
		desc.numFaces = (int)vf.size();
		desc.numVertsPerFace = vf.data();
		desc.vertIndicesPerFace = f.data();
		if (!e.empty()) {
			desc.numCreases = (int)e.size() / 2;
			desc.creaseWeights = w.data();
			desc.creaseVertexIndexPairs = e.data();
		}
		if (!p.empty()) {
			desc.numCorners = (int)p.size();
			desc.cornerWeights = &w[e.size() / 2];
			desc.cornerVertexIndices = p.data();
		}
		desc.numFVarChannels = 1;
		desc.fvarChannels = &fvar;

		// create refiner
		Sdc::Options opt;
		opt.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_AND_CORNER);

		Far::TopologyRefiner* refiner(
			Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Create(desc,
				Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Options(
					Sdc::SchemeType::SCHEME_CATMARK, opt)));

		int numVertexElements = 3; // x, y, z
		//int numVaryingElements = 3; // normal

		shader_osd_info.elem.bits.fvar_width = 2;
		shader_osd_info.elem.bits.is_adaptive = 1;
		shader_osd_info.elem.bits.patch_cull = 1;
		shader_osd_info.elem.bits.screen_space_tess = 1;
		shader_osd_info.elem.bits.single_crease_patch = 1;
		shader_osd_info.elem.bits.fractional = 1;
		shader_osd_info.elem.Set_patch_type(default_patch_type);

		Osd::MeshBitset bits;
		bits.set(Osd::MeshAdaptive, shader_osd_info.elem.bits.is_adaptive == 1);           // set adaptive
		bits.set(Osd::MeshEndCapBSplineBasis, shader_osd_info.elem.Get_patch_type() == Far::PatchDescriptor::Type::REGULAR); // use b-spline basis patch for endcap.
		bits.set(Osd::MeshEndCapGregoryBasis, shader_osd_info.elem.Get_patch_type() == Far::PatchDescriptor::Type::GREGORY_BASIS);
		bits.set(Osd::MeshUseSingleCreasePatch, shader_osd_info.elem.bits.single_crease_patch == 1);
		bits.set(Osd::MeshFVarData, shader_osd_info.elem.bits.fvar_width != 0);

		mesh.reset(new Osd::Mesh<
			Osd::GLVertexBuffer, Osd::GLStencilTableSSBO,
			Osd::GLComputeEvaluator, Osd::GLPatchTable>
			(
				refiner,
				numVertexElements,
				0,
				glm::clamp(default_patch, 0, max_patch),
				bits,
				&evaluator
				)
			);

		// Face Per Material
		GLuint matbuffer;
		glCreateBuffers(1, &matbuffer);
		glNamedBufferData(matbuffer, mpf.size() * sizeof(int), mpf.data(), GL_STATIC_DRAW);
		glCreateTextures(GL_TEXTURE_BUFFER, 1, &material_index_texture);
		glTextureBuffer(material_index_texture, GL_R32I, matbuffer);
		glDeleteBuffers(1, &matbuffer);

		// Face-Varying
		Far::PrimvarRefiner primvar(*refiner);
		std::vector<glm::vec2> fvaruv(uv_pool);
		fvaruv.resize(refiner->GetNumFVarValuesTotal(0));
		FVarUV* src = reinterpret_cast<FVarUV*>(fvaruv.data());
		for (int level = 1; level <= refiner->GetMaxLevel(); ++level) {
			FVarUV* dst = src + refiner->GetLevel(level - 1).GetNumFVarValues(0);
			primvar.InterpolateFaceVarying(level, src, dst, 0);
			src = dst;
		}
		Far::ConstIndexArray indices = mesh->GetFarPatchTable()->GetFVarValues(0);
		std::vector<glm::vec2> uv_data; uv_data.reserve(indices.size());// *FVarUV::GetWidth());
		for (int i = 0; i < indices.size(); ++i) {
			//int index = indices[i];// *FVarUV::GetWidth();
			//for (int j = 0; j < FVarUV::GetWidth(); ++j) {
			uv_data.emplace_back(fvaruv[indices[i]]);// [j]);
			//}
		}
		GLuint fvar_buffer;
		glCreateBuffers(1, &fvar_buffer);
		glNamedBufferData(fvar_buffer, uv_data.size() * sizeof(glm::vec2), uv_data.data(), GL_STATIC_DRAW);
		glCreateTextures(GL_TEXTURE_BUFFER, 1, &texcoord_fvar_texture);
		glTextureBuffer(texcoord_fvar_texture, GL_R32F, fvar_buffer);
		glDeleteBuffers(1, &fvar_buffer);

		Update(v.data(), 0, (int)v.size() / 3);

		if (vao)
			glDeleteVertexArrays(1, &vao);
		glCreateVertexArrays(1, &vao);

		// インデックスバッファ
		glVertexArrayElementBuffer(vao, mesh->GetPatchTable()->GetPatchIndexBuffer());
		// 頂点バッファ
		const glslProgram& program = shader_manage->Get(default_glsl_info, shader_osd_info);
		GLuint posIndex = program.GetAttrib("position")->GetIndex(),
			vertbuf = mesh->BindVertexBuffer();
			//varybuf = mesh->BindVaryingBuffer();
		
		// Vertex Position
		glVertexArrayVertexBuffer(vao, 0, vertbuf, 0, sizeof(float) * numVertexElements);
		glVertexArrayAttribFormat(vao, posIndex, numVertexElements, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao, posIndex, 0);
		glEnableVertexArrayAttrib(vao, posIndex);
		
		// パッチパラメータバッファ
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_BUFFER, mesh->GetPatchTable()->GetPatchParamTextureBuffer());
		glBindTextureUnit(0, mesh->GetPatchTable()->GetPatchParamTextureBuffer());
		//glBindTextureUnit(1, mesh->GetPatchTable()->GetPatchIndexTextureBuffer());

		glm::mat4 trans = glm::translate(glm::vec3(
			location[0].get<double>(), location[1].get<double>(), location[2].get<double>()));

		glm::mat4 rot =  glm::eulerAngleXYZ(
			((float)rotation[0].get<double>()),
			((float)rotation[1].get<double>()),
			((float)rotation[2].get<double>()));
		
		glm::mat4 sc = glm::scale(glm::vec3(
			scale[0].get<double>(), scale[1].get<double>(), scale[2].get<double>()));

		modelMatrix = trans * rot * sc;
	}


	void model::Update(const float* v1, int start1, int size1, const float* v2, int start2, int size2) {
		mesh->UpdateVertexBuffer(v1, start1, size1);
		if (v2)
			mesh->UpdateVaryingBuffer(v2, start2, size2);
		mesh->Refine();
		mesh->Synchronize();
	}

	void model::Draw()
	{
		using namespace OpenSubdiv;

		if (shader_osd_info.elem.bits.patch_cull == 0)
			glDisable(GL_CULL_FACE);
		mesh->BindVertexBuffer();
		if (mesh->GetPatchTable()->GetPatchParamTextureBuffer())
			glBindTextureUnit(0, mesh->GetPatchTable()->GetPatchParamTextureBuffer());
		//if (mesh->GetPatchTable()->GetPatchIndexTextureBuffer())
		//	glBindTextureUnit(1, mesh->GetPatchTable()->GetPatchIndexTextureBuffer());
		if (texcoord_fvar_texture)
			glBindTextureUnit(1, texcoord_fvar_texture);
		if (material_index_texture)
			glBindTextureUnit(2, material_index_texture);
		if (material->GetTexture())
			glBindTextureUnit(3, material->GetTexture());
		if (default_texture)
			glBindTextureUnit(4, default_texture);
		glBindVertexArray(vao);
		for (int i = 0; i < mesh->GetPatchTable()->GetPatchArrays().size(); ++i) {
			Osd::PatchArray const & patch = mesh->GetPatchTable()->GetPatchArrays()[i];
			Far::PatchDescriptor desc = patch.GetDescriptor();

			int numVertsPerPatch = desc.GetNumControlVertices();  // 16 for B-spline patches
			osd_info info(shader_osd_info);
			info.elem.Set_patch_type(desc.GetType());
			const glslProgram& program = shader_manage->Get(default_glsl_info, info);
			GLuint prog = program.GetProgram();
			glUseProgram(prog); 
			glProgramUniform1i(prog, program.GetUniform("OsdPatchParamBuffer")->GetIndex(), 0);
			glProgramUniform1i(prog, program.GetUniform("OsdFVarDataBuffer")->GetIndex(), 1);
			glProgramUniform1i(prog, program.GetUniform("MaterialIndexBuffer")->GetIndex(), 2);
			glProgramUniform1i(prog, program.GetUniform("MaterialDataBuffer")->GetIndex(), 3);
			glProgramUniform1i(prog, program.GetUniform("diffuse_map")->GetIndex(), 4);
			glProgramUniform1i(prog, program.GetUniform("PrimitiveIdBase")->GetIndex(), patch.GetPrimitiveIdBase());
			glProgramUniformMatrix4fv(prog, program.GetUniform("model")->GetIndex(), 1, GL_FALSE, &modelMatrix[0][0]);
			glPatchParameteri(GL_PATCH_VERTICES, numVertsPerPatch);
			glDrawElements(GL_PATCHES, patch.GetNumPatches() * numVertsPerPatch,
				GL_UNSIGNED_INT, (void *)(patch.GetIndexBase() * sizeof(unsigned int)));
			++draw_call;
		}
		glBindVertexArray(0);
		glUseProgram(0);

		if (shader_osd_info.elem.bits.patch_cull == 0)
			glEnable(GL_CULL_FACE);
	}
	const glslProgram * model::GetProgram() const
	{
		return program.get();
	}
	void model::SetProgram(std::shared_ptr<glslProgram> prog)
	{
		if (prog)
			program = prog;
	}

	int model::draw_call = 0;
	int model::max_patch = 6;
	int model::default_patch = 2;
	OpenSubdiv::Far::PatchDescriptor::Type model::default_patch_type;
	GLuint model::default_texture = 0;
	glslProgram::glsl_info model::default_glsl_info;
	ShaderManager* model::shader_manage;
	OpenSubdiv::Osd::EvaluatorCacheT<OpenSubdiv::Osd::GLComputeEvaluator> model::evaluator;
}

