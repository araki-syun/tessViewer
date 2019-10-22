#pragma once

#include <vector>
#include <stdexcept>
#include <memory>

#include <opensubdiv\osd\glMesh.h>

namespace glapp {
class mesh {
	friend class model;

public:
	mesh(void);
	virtual ~mesh(void);
	explicit            operator bool() const;
	int                 GetNumVertex(void) const;
	int                 GetNumEdgeIndex(void) const;
	int                 GetNumFaceIndex(void) const;
	int                 GetNumElement(void) const;
	const unsigned int* GetEdgeIndexPointer(void) const;
	const unsigned int* GetFaceIndexPointer(void) const;
	const float*        GetVertexPointer(void) const;
	void                Bind();
	void                Draw();

	//	virtual void Load(const std::string& filename);
protected:
	void Refine();

protected:
	std::vector<float>                                _vertices;
	std::vector<int>                                  _faceIndices;
	std::vector<int>                                  _numVertPerFace;
	std::vector<unsigned int>                         _edgeIndices;
	std::vector<float>                                _sharpnessValues;
	int                                               _numElement;
	std::unique_ptr<OpenSubdiv::Osd::GLMeshInterface> _osdMesh;
};
} // namespace glapp
