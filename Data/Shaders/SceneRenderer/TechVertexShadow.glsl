#version 330 core

uniform mat4 uModelMtx;

layout (location = 0) in  vec3 pos;

out vec3 position;

void main(void)	
{
	position= pos;
	gl_Position	= uModelMtx * vec4(pos, 1.0);
}