
#version 460 core

layout (location = 0) in vec2 aPosition;

uniform sampler2D u_RSMRadiantFluxTexture;
uniform sampler2D u_RSMPositionTexture;
uniform sampler2D u_RSMNormalTexture;

uniform float u_CellSize;
uniform vec3 u_MinAABB;
uniform int u_RSMResolution;
uniform vec3 u_Dimensions;

out VertexData
{
	vec3 Normal;
	vec4 flux;
	flat ivec3 cellIndex;
}vs_out;


ivec3 getCellIndx(vec3 pos,vec3 N)
{
	return ivec3((pos-u_MinAABB)/u_CellSize+ 0.5 * N);
}

ivec3 convertPointToGridIndex(vec3 vPos) 
{
	return ivec3((vPos - u_MinAABB) / u_CellSize);
}

void main()
{

	ivec2 RSMCoords = ivec2(gl_VertexID%u_RSMResolution,  gl_VertexID/u_RSMResolution);
	vec3 worldPos = texelFetch(u_RSMPositionTexture, RSMCoords,0).rgb;
	vec3 worldNormal = texelFetch(u_RSMNormalTexture, RSMCoords,0).xyz;
	vec4 flux = texelFetch(u_RSMRadiantFluxTexture, RSMCoords, 0);

	ivec3 cellIndex = getCellIndx(worldPos, worldNormal);
	//ivec3 cellIndex = convertPointToGridIndex(worldPos);

	vs_out.Normal = worldNormal;
	vs_out.flux = flux;
	vs_out.cellIndex = cellIndex;

	vec2 ndc = (vec2(cellIndex.xy)+ 0.5) / u_Dimensions.xy * 2.0 - 1.0;

	gl_Position = vec4(ndc, 0.0, 1.0);
	//gl_Position = vec4(aPosition, 0.0, 1.0);
	//gl_Position = vec4(0.5f, 0.5f, 0.0f, 1.0f);
	gl_PointSize = 1.0;

}