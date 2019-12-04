#pragma once

#include "opensubdiv\osd\opengl.h"
#include "opensubdiv\osd\glslPatchShaderSource.h"
#include "opensubdiv\far\patchDescriptor.h"

struct OsdInfo {
	enum ElemType {
		FVAR_WIDTH,
		PATCH_TYPE,
		NUM_PRIM_PER_VERTEX,
		IS_ADAPTIVE,
		SCREEN_SPACE_TESS,
		FRACTIONAL,
		PATCH_CULL,
		SINGLE_CREASE_PATCH
	};
	union Element {
		struct {
			unsigned int fvar_width : 8;
			unsigned int patch_type : 4;
			unsigned int num_prim_per_vertex : 4;
			unsigned int is_adaptive : 1;
			unsigned int screen_space_tess : 1;
			unsigned int fractional : 1;
			unsigned int patch_cull : 1;
			unsigned int single_crease_patch : 1;
		} bits;
		unsigned int data;

		bool                                   operator==(Element e) const;
		OpenSubdiv::Far::PatchDescriptor::Type GetPatchType() const;
		void SetPatchType(OpenSubdiv::Far::PatchDescriptor::Type type);
	} elem{};

	OsdInfo();
	OsdInfo(const OsdInfo& info);
	void        Set(ElemType e, unsigned int value);
	std::string Str() const;
};
