
#ifdef VERTEX_SHADER

/*
#undef OSD_USER_VARYING_DECLARE
#define OSD_USER_VARYING_DECLARE \
vec3 normal;

#undef OSD_USER_VARYING_ATTRIBUTE_DECLARE
#define OSD_USER_VARYING_ATTRIBUTE_DECLARE \
in vec3 normal;

#undef OSD_USER_VARYING_PER_VERTEX
#define OSD_USER_VARYING_PER_VERTEX() \
    outpt.normal = mat3(transpose(inverse(view * model))) * normal

#undef OSD_USER_VARYING_PER_CONTROL_POINT
#define OSD_USER_VARYING_PER_CONTROL_POINT(ID_OUT, ID_IN) \
    outpt[ID_OUT].normal = inpt[ID_IN].normal

#undef OSD_USER_VARYING_PER_EVAL_POINT
#define OSD_USER_VARYING_PER_EVAL_POINT(UV, a, b, c, d) \
    outpt.color = \
        mix(mix(inpt[a].normal, inpt[b].normal, UV.x), \
            mix(inpt[c].normal, inpt[d].normal, UV.x), UV.y)*/

/*
in vec4 attrib_pos;
in vec3 attrib_nor;

out vec3 vpos;
out vec3 vnor;

void main(){
	mat4 mvp = projection * view * model;
	vpos = (mvp * position).xyz;
	mat3 normalMatrix = mat3(transpose(inverse(view * model)));
	vnor = normalMatrix * attrib_nor;
	gl_Position = mvp * position;
	
}
*/

#endif

