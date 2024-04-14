#version 430 core
layout(location = 0) in vec3 _Position;
layout(location = 1) in vec3 _Normal;
layout(location = 2) in vec2 _TexCoord;

uniform mat4 model;

out VS_OUT{
    vec2 TexCoords;
    vec3 Normal;
} vs_out;



void main()
{
    vs_out.Normal = normalize(transpose(inverse(mat3(model))) * _Normal);
    vs_out.TexCoords = _TexCoord;
    gl_Position = model * vec4(_Position, 1.0);
}  