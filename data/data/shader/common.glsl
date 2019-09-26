

#undef OSD_USER_VARYING_DECLARE
#define OSD_USER_VARYING_DECLARE \
vec2 tex_Coord;

/*
#undef OSD_USER_VARYING_ATTRIBUTE_DECLARE
#define OSD_USER_VARYING_ATTRIBUTE_DECLARE \
in vec3 
*/
/*
#undef OSD_USER_VARYING_PER_VERTEX
#define OSD_USER_VARYING_PER_VERTEX() \
	outpt.tex_Coord = tex_Coord;
*/

#undef OSD_USER_VARYING_PER_CONTROL_POINT
#define OSD_USER_VARYING_PER_CONTROL_POINT(ID_OUT, ID_IN) \
    outpt[ID_OUT].tex_Coord = inpt[ID_IN].tex_Coord

#undef OSD_USER_VARYING_PER_EVAL_POINT
#define OSD_USER_VARYING_PER_EVAL_POINT(UV, a, b, c, d) \
    outpt.tex_Coord = \
        mix(mix(inpt[a].tex_Coord, inpt[b].tex_Coord, UV.x), \
            mix(inpt[c].tex_Coord, inpt[d].tex_Coord, UV.x), UV.y)



uniform mat4 model;

layout(std140) uniform Transform{
	mat4 view;
	mat4 projection;
};

layout(std140) uniform Tessellation {
    float TessLevel;
};

uniform int GregoryQuadOffsetBase;
uniform int PrimitiveIdBase;

uniform isamplerBuffer OsdPatchIndexBuffer;


mat4 OsdModelViewMatrix()
{
    return view * model;
}
mat4 OsdProjectionMatrix()
{
    return projection;
}
mat4 OsdModelViewProjectionMatrix()
{
    return projection * view * model;
}
float OsdTessLevel()
{
    return TessLevel;
}
int OsdGregoryQuadOffsetBase()
{
    return GregoryQuadOffsetBase;
}
int OsdPrimitiveIdBase()
{
    return PrimitiveIdBase;
}
int OsdBaseVertex()
{
    return 0;
}


