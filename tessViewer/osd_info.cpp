#pragma once

#include "osd_info.h"

#include <sstream>

OsdInfo::OsdInfo() {
	elem.bits.fvar_width = 0;
	elem.bits.patch_type = OpenSubdiv::Far::PatchDescriptor::Type::NON_PATCH;
	elem.bits.num_prim_per_vertex = 3;
	elem.bits.is_adaptive         = 1;
	elem.bits.screen_space_tess   = 1;
	elem.bits.fractional          = 0;
	elem.bits.patch_cull          = 0;
	elem.bits.single_crease_patch = 0;
}

OsdInfo::OsdInfo(const OsdInfo& info) { elem.data = info.elem.data; }

void OsdInfo::Set(ElemType e, unsigned int value) {
	switch (e) {
	case OsdInfo::FVAR_WIDTH: elem.bits.fvar_width = value; break;
	case OsdInfo::PATCH_TYPE: elem.bits.patch_type = value; break;
	case OsdInfo::NUM_PRIM_PER_VERTEX:
		elem.bits.num_prim_per_vertex = value;
		break;
	case OsdInfo::IS_ADAPTIVE:
		elem.bits.is_adaptive = (value != 0u ? 1 : 0);
		break;
	case OsdInfo::SCREEN_SPACE_TESS:
		elem.bits.screen_space_tess = (value != 0u ? 1 : 0);
		break;
	case OsdInfo::FRACTIONAL:
		elem.bits.fractional = (value != 0u ? 1 : 0);
		break;
	case OsdInfo::PATCH_CULL:
		elem.bits.patch_cull = (value != 0u ? 1 : 0);
		break;
	case OsdInfo::SINGLE_CREASE_PATCH:
		elem.bits.single_crease_patch = (value != 0u ? 1 : 0);
		break;
	default: break;
	}
}

std::string OsdInfo::Str() const {
	std::stringstream ss;
	ss << std::hex << elem.data;
	return ss.str();
}

bool OsdInfo::Element::operator==(const OsdInfo::Element e) const {
	return data == e.data;
}

OpenSubdiv::Far::PatchDescriptor::Type OsdInfo::Element::GetPatchType() const {
	return OpenSubdiv::Far::PatchDescriptor::Type(bits.patch_type);
}

void OsdInfo::Element::SetPatchType(
	OpenSubdiv::Far::PatchDescriptor::Type type) {
	bits.patch_type = type;
}
