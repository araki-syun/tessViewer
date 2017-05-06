#pragma once

#include "osd_info.h"

#include <sstream>

osd_info::osd_info()
{
	elem.bits.fvar_width = 0;
	elem.bits.patch_type = OpenSubdiv::Far::PatchDescriptor::Type::NON_PATCH;
	elem.bits.num_prim_per_vertex = 3;
	elem.bits.is_adaptive = 1;
	elem.bits.screen_space_tess = 1;
	elem.bits.fractional = 0;
	elem.bits.patch_cull = 0;
	elem.bits.single_crease_patch = 0;
}

osd_info::osd_info(const osd_info & info)
{
	elem.data = info.elem.data;
}

void osd_info::set(Elem_Type e, unsigned int value)
{
	switch (e)
	{
	case osd_info::FVAR_WIDTH:
		elem.bits.fvar_width = value;
		break;
	case osd_info::PATCH_TYPE:
		elem.bits.patch_type = value;
		break;
	case osd_info::NUM_PRIM_PER_VERTEX:
		elem.bits.num_prim_per_vertex = value;
		break;
	case osd_info::IS_ADAPTIVE:
		elem.bits.is_adaptive = (value ? 1 : 0);
		break;
	case osd_info::SCREEN_SPACE_TESS:
		elem.bits.screen_space_tess = (value ? 1 : 0);
		break;
	case osd_info::FRACTIONAL:
		elem.bits.fractional = (value ? 1 : 0);
		break;
	case osd_info::PATCH_CULL:
		elem.bits.patch_cull = (value ? 1 : 0);
		break;
	case osd_info::SINGLE_CREASE_PATCH:
		elem.bits.single_crease_patch = (value ? 1 : 0);
		break;
	default:
		break;
	}
}

const std::string osd_info::str() const
{
	std::stringstream ss;
	ss << std::hex << elem.data;
	return ss.str();
}

bool osd_info::Element::operator==(const osd_info::Element e) const
{
	return data == e.data;
}

OpenSubdiv::Far::PatchDescriptor::Type osd_info::Element::Get_patch_type() const
{
	return OpenSubdiv::Far::PatchDescriptor::Type(bits.patch_type);
}

void osd_info::Element::Set_patch_type(OpenSubdiv::Far::PatchDescriptor::Type type)
{
	bits.patch_type = type;
}


