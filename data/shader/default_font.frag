#version 450

in vec2 uv;

uniform sampler2D texture;
uniform vec3 font_color;

out vec4 fragment;

void main(){
	float alpha = texture2D(texture, uv).r;
	if(alpha == 0.0)
		discard;
	//fragment = vec4(font_color.rg, alpha, 1.0);
	fragment = vec4(font_color * alpha, 1.0);
}


