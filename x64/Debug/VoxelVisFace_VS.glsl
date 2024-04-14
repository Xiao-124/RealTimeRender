#version 430 core

layout(location = 0) in vec3 v_position;

layout(std140, binding = 0) uniform u_Matrices4ProjectionWorld
{
	mat4 u_ProjectionMatrix;
	mat4 u_ViewMatrix;
};



out vec3 f_world_pos;

void main()
{
	f_world_pos = v_position;
	gl_Position = u_ProjectionMatrix*u_ViewMatrix * vec4(f_world_pos, 1.0f);
}