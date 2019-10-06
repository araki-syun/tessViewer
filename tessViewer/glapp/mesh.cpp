#include "mesh.h"

#include <assert.h>

#include <GL\glew.h>

#include <opensubdiv\far\topologyDescriptor.h>

//	cpu
#include <opensubdiv\far\stencilTable.h>
#include <opensubdiv\osd\cpuGLVertexBuffer.h>
#include <opensubdiv\osd\cpuEvaluator.h>
#include <opensubdiv\osd\glPatchTable.h>
//	gpu
#include <opensubdiv\osd\glVertexBuffer.h>
#include <opensubdiv\osd\glComputeEvaluator.h>
#include <opensubdiv\osd\opengl.h>

#include "useConfig.h"

using namespace OpenSubdiv;

namespace glapp{
	mesh::mesh(void)
	{
		_numElement = 0;
	}


	mesh::~mesh(void)
	{
	}


	int mesh::GetNumVertex(void) const
	{
		assert(_numElement);
		return _vertices.size() / _numElement;
	}


	int mesh::GetNumFaceIndex(void) const
	{
		return _faceIndices.size();
	}


	int mesh::GetNumElement(void) const
	{
		assert(_numElement);
		return _numElement;
	}


	const unsigned int* mesh::GetEdgeIndexPointer(void) const{
		if(_edgeIndices.empty())
			return NULL;
		return _edgeIndices.data();
	}

	const unsigned int* mesh::GetFaceIndexPointer(void) const
	{
		if(_faceIndices.empty())
			return NULL;
		return reinterpret_cast<const unsigned int*>(_faceIndices.data());
	}


	const float* mesh::GetVertexPointer(void) const
	{
		if(_vertices.empty())
			return NULL;
		return _vertices.data();
	}


	int mesh::GetNumEdgeIndex(void) const
	{
		return _edgeIndices.size();
	}


	//void mesh::Load(const std::string& filename){
	//	std::string err = "Mesh Load ERROR : ";
	//	throw std::logic_error(err += filename);
	//}

	void mesh::Refine(){
		Far::TopologyDescriptor desc;

		desc.numVertices = _vertices.size() / 3;
		desc.numFaces = _numVertPerFace.size();
		desc.numVertsPerFace = _numVertPerFace.data();
		desc.vertIndicesPerFace = _faceIndices.data();

		Far::TopologyRefiner* refiner = Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Create(desc);

		int numVertexElements = 3; // x, y, z

		Osd::MeshBitset bits;

		bits.set(Osd::MeshBits::MeshAdaptive, true);
		bits.set(Osd::MeshBits::MeshEndCapBSplineBasis, true);

		_osdMesh.reset(new Osd::Mesh<Osd::CpuGLVertexBuffer, Far::StencilTable, Osd::CpuEvaluator, Osd::GLPatchTable>(
			refiner, numVertexElements, 0, config.Get<int>("OSD.PatchLevel"), bits));

		_osdMesh->UpdateVertexBuffer(_vertices.data(), 0, _vertices.size() / numVertexElements);
		_osdMesh->Refine();
	}

	void mesh::Bind(){
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _osdMesh->GetPatchTable()->GetPatchIndexBuffer());

		int numVertexElements = 3;
		glBindBuffer(GL_ARRAY_BUFFER, _osdMesh->BindVertexBuffer());
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, numVertexElements, GL_FLOAT, GL_FALSE, numVertexElements * sizeof(float), nullptr);

		glActiveTexture(GL_TEXTURE0);
		glBindBuffer(GL_TEXTURE_BUFFER, _osdMesh->GetPatchTable()->GetPatchParamTextureBuffer());
	}

	void mesh::Draw(){
		// 使用していない
		/*const Osd::PatchArrayVector &pav = _osdMesh->GetPatchTable()->GetPatchArrays();
		for(Osd::PatchArray pa : pav){
			Far::PatchDescriptor desc = pa.GetDescriptor();
			int numVertPerPatch = desc.GetNumControlVertices();
			glUseProgram();
			glPatchParameteri(GL_PATCH_VERTICES, numVertPerPatch);
			glDrawElements(GL_PATCHES, pa.GetNumPatches() * numVertPerPatch, GL_UNSIGNED_INT, nullptr);
		}*/
	}
}
