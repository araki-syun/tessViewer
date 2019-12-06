

#ifdef FRAGMENT_SHADER

uniform samplerBuffer MaterialDataBuffer;
uniform isamplerBuffer MaterialIndexBuffer;
uniform sampler2D diffuse_map;

layout(std140) uniform MaterialOffset{
	int offset;
};

in block {
	OutputVertex v;
	noperspective vec4 edgeDistance;
#if defined OSD_PATCH_ENABLE_SINGLE_CREASE
	vec2 vSegments;
#endif
	OSD_USER_VARYING_DECLARE
} inpt;


out vec4 fragment;

layout(std140) uniform LightInfo{
	vec3 pos;
	vec3 vector;
	vec3 ia;
	vec3 id;
	vec3 is;
};
struct Phong{
	vec3 ka;
	vec3 kd;
	vec3 ks;
	float shine;
};

void main(){
	int patchIndex = OsdGetPatchIndex(gl_PrimitiveID);
	int faceId = //int(inpt.v.patchCoord.w);
	//	texelFetch(OsdPatchIndexBuffer, patchIndex).x;
		OsdGetPatchFaceId(OsdGetPatchParam(patchIndex));
	int matIndex = texelFetch(MaterialIndexBuffer, faceId).x * offset;
	// * textureSize(OsdFVarDataBuffer);
	//matIndex = offset;
	Phong mat;
	mat.ka = vec3(
		texelFetch(MaterialDataBuffer, matIndex).r,
		texelFetch(MaterialDataBuffer, matIndex+1).r,
		texelFetch(MaterialDataBuffer, matIndex+2).r);
	mat.kd = vec3(
		texelFetch(MaterialDataBuffer, matIndex+3).r,
		texelFetch(MaterialDataBuffer, matIndex+4).r,
		texelFetch(MaterialDataBuffer, matIndex+5).r);
	mat.ks = vec3(
		texelFetch(MaterialDataBuffer, matIndex+6).r,
		texelFetch(MaterialDataBuffer, matIndex+7).r,
		texelFetch(MaterialDataBuffer, matIndex+8).r);
	mat.shine = texelFetch(MaterialDataBuffer, matIndex+9).r;
	
	/*if(inpt.v.patchCoord.z > 1.0)
		mat.kd = vec3(0.8, 0.8, 0.8);
	if(inpt.v.patchCoord.z > 2.0)
		mat.kd = vec3(0.2, 0.2, 0.8);
	if(inpt.v.patchCoord.z > 3.0)
		mat.kd = vec3(0.2, 0.8, 0.2);
	if(inpt.v.patchCoord.z > 4.0)
		mat.kd = vec3(0.8, 0.5, 0.2);
	if(inpt.v.patchCoord.z > 5.0)
		mat.kd = vec3(0.8, 0.2, 0.2);*/
	
	vec3 L = normalize(pos - inpt.v.position.xyz);
	vec3 N = normalize(inpt.v.normal);
	vec3 R = reflect(-L, N);
	vec3 V = normalize(-inpt.v.position.xyz);
	
	vec3 kd = texture2D(diffuse_map, inpt.tex_Coord).rgb;
	
	vec3 amb = kd * ia;
	vec3 dif = kd * id * max(dot(N, L), 0.0);
	vec3 spec = mat.ks * is * pow(max(dot(R, N), 0.0), mat.shine);
	
	// Edge Line Shade
	float d = min(inpt.edgeDistance[0],min(inpt.edgeDistance[1],inpt.edgeDistance[2]));
	float val = smoothstep(0.0, 1.5, d);
	
	//dif = vec3(inpt.tex_Coord,0);
	
	fragment = mix(vec4(vec3(0),1), vec4(amb + dif + spec, 1.0), val);
}

#endif

