
#ifdef GEOMETRY_SHADER

#ifdef PRIM_QUAD

	layout(lines_adjacency) in;

	#define EDGE_VERTS 4

#endif // PRIM_QUAD

#ifdef	PRIM_TRI

	layout(triangles) in;

	#define EDGE_VERTS 3

#endif // PRIM_TRI

layout(triangle_strip, max_vertices = EDGE_VERTS) out;

in block {
	OutputVertex v;
#if defined OSD_PATCH_ENABLE_SINGLE_CREASE
	vec2 vSegments;
#endif
	OSD_USER_VARYING_DECLARE
} inpt[EDGE_VERTS];

out block{
	OutputVertex v;
	noperspective out vec4 edgeDistance;
#ifdef OSD_PATCH_ENABLE_SINGLE_CREASE
	vec2 vSegments;
#endif
	OSD_USER_VARYING_DECLARE
} outpt;

void emit(int index, vec3 normal){
	outpt.v.position = inpt[index].v.position;
	outpt.v.patchCoord = inpt[index].v.patchCoord;
	outpt.v.normal = inpt[index].v.normal;
	//outpt.v.normal = normal;
#ifdef OSD_PATCH_ENABLE_SINGLE_CREASE
	outpt.vSegments = inpt[index].vSegments;
#endif
	
	vec2 uv;
	OSD_COMPUTE_FACE_VARYING_2(uv, 0, inpt[index].v.tessCoord);
	outpt.tex_Coord = uv;
	
	gl_Position = OsdProjectionMatrix() * inpt[index].v.position;
	EmitVertex();
}

const float VIEWPORT_SCALE = 1024.0; // XXXdyu

float edgeDistance(vec4 p, vec4 p0, vec4 p1)
{
	return VIEWPORT_SCALE *
		abs((p.x - p0.x) * (p1.y - p0.y) -
			(p.y - p0.y) * (p1.x - p0.x)) / length(p1.xy - p0.xy);
}

void emit(int index, vec3 normal, vec4 edgeVerts[EDGE_VERTS])
{
	outpt.edgeDistance[0] =
		edgeDistance(edgeVerts[index], edgeVerts[0], edgeVerts[1]);
	outpt.edgeDistance[1] =
		edgeDistance(edgeVerts[index], edgeVerts[1], edgeVerts[2]);
#ifdef PRIM_TRI
	outpt.edgeDistance[2] =
		edgeDistance(edgeVerts[index], edgeVerts[2], edgeVerts[0]);
#endif
#ifdef PRIM_QUAD
	outpt.edgeDistance[2] =
		edgeDistance(edgeVerts[index], edgeVerts[2], edgeVerts[3]);
	outpt.edgeDistance[3] =
		edgeDistance(edgeVerts[index], edgeVerts[3], edgeVerts[0]);
#endif

	emit(index, normal);
}

void main(){
	gl_PrimitiveID = gl_PrimitiveIDIn;
	
	vec3 A = (inpt[1].v.position - inpt[0].v.position).xyz;
	vec3 B = (inpt[2].v.position - inpt[0].v.position).xyz;
	vec3 n0 = normalize(cross(B, A));
	
	vec4 edgeVerts[EDGE_VERTS];
	edgeVerts[0] = OsdProjectionMatrix() * inpt[0].v.position;
	edgeVerts[1] = OsdProjectionMatrix() * inpt[1].v.position;
	edgeVerts[2] = OsdProjectionMatrix() * inpt[2].v.position;

	edgeVerts[0].xy /= edgeVerts[0].w;
	edgeVerts[1].xy /= edgeVerts[1].w;
	edgeVerts[2].xy /= edgeVerts[2].w;

	emit(0, n0, edgeVerts);
	emit(1, n0, edgeVerts);
	emit(2, n0, edgeVerts);
	
	EndPrimitive();
}

#endif

