#version 450

in vec2 position;
in vec2 uv_coord;

out vec2 uv;

void main(){
	uv = uv_coord;
	gl_Position = vec4(position, 0.0, 1.0);
}


