#version 430 core

layout (location = 0) in vec2 _Position;
layout (location = 1) in vec2 _TexCoords;

uniform int ilayer;

out vec2 v2f_TexCoords;
flat out int layer;
void main()
{
	layer = ilayer;
	gl_Position = vec4(_Position, 0.0, 1.0);
	v2f_TexCoords = _TexCoords;
}