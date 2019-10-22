#include "model.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <functional>
#include <boost\format.hpp>

#include <opensubdiv\far\topologyDescriptor.h>

using namespace OpenSubdiv;

namespace glapp {
//const model::model_node::mesh_node_value* model::model_node::operator->() const{
//	return &value;
//}

std::unordered_map<std::string, std::shared_ptr<mesh>>     model::mesh_list;
std::unordered_map<std::string, std::shared_ptr<material>> model::material_list;

model::model(void) {}

model::~model(void) {}

void model::Load(const std::string& filename) {
	Assimp::Importer imp;
	const aiScene*   scene =
		imp.ReadFile(filename, aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw std::runtime_error(
			(boost::format("Model Load ERROR : %1%") % filename).str());

	//mesh ÇÃì«Ç›çûÇ›

	int numMesh = 0;

	std::function<void(aiNode*)> countMesh = [&numMesh](aiNode* node) {
		for (int i = 0; i < node->mNumChildren; ++i) {}
	};
}

void model::node_Draw(const model::model_node* n) const {
	{
		const Osd::PatchArrayVector& pav =
			(*n)->pMesh->_osdMesh->GetPatchTable()->GetPatchArrays();
		glUseProgram((*n)->pMaterial->GetGLSLProgram()->GetProgram());
		(*n)->pMaterial->Uniform();
		for (Osd::PatchArray pa : pav) {
			Far::PatchDescriptor desc            = pa.GetDescriptor();
			int                  numVertPerPatch = desc.GetNumControlVertices();

			glPatchParameteri(GL_PATCH_VERTICES, numVertPerPatch);
			glDrawElements(GL_PATCHES, pa.GetNumPatches() * numVertPerPatch,
						   GL_UNSIGNED_INT, nullptr);
		}
		glUseProgram(0);
	}
	for (std::shared_ptr<model_node> n : n->children) {
		node_Draw(n.get());
	}
}

void model::Draw() const { node_Draw(&root_mesh); }
} // namespace glapp
