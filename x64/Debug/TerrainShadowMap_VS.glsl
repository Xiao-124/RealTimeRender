#version 430 core

uniform float gTerrainResolution;
uniform sampler2D TerrainHeightmapTex;
uniform mat4 gViewProjMat;
uniform float gScreenshotCaptureActive;

out vec4 vs_position;
out vec4 vs_Uvs;
out vec4 vs_WorldPos;
out vec3 vs_color;
out vec3 vs_normal;

vec4 SampleTerrain(in float quadx, in float quady, in vec3 qp)
{
	const float terrainWidth = 100.0f;	// 100 km edge
	const float maxTerrainHeight = 100.0f;
	const float quadWidth = terrainWidth / gTerrainResolution;

	vec2 Uvs = (vec2(quadx, quady) + qp.xy) / gTerrainResolution;
#if 0
	const float height = textureLod(TerrainHeightmapTex, Uvs, 0).r;
#else
	const float offset = 0.0008;
	float HeightAccum = textureLod(TerrainHeightmapTex, Uvs + vec2(0.0f, 0.0f), 0).r;
	HeightAccum+= textureLod(TerrainHeightmapTex, Uvs + vec2( offset, 0.0f), 0).r;
	HeightAccum+= textureLod(TerrainHeightmapTex, Uvs + vec2(-offset, 0.0f), 0).r;
	HeightAccum+= textureLod(TerrainHeightmapTex, Uvs + vec2( 0.0f, offset), 0).r;
	HeightAccum+= textureLod(TerrainHeightmapTex, Uvs + vec2( 0.0f,-offset), 0).r;
	const float height = HeightAccum / 5;
#endif
	vec4 WorldPos = vec4((vec3(quadx, quady, maxTerrainHeight * height) + qp) * quadWidth - 0.5f * vec3(terrainWidth, terrainWidth, 0.0), 1.0f);
	WorldPos.xyz += vec3(-terrainWidth * 0.45, 0.4*terrainWidth, -0.0f);	// offset to position view
	return WorldPos;
}

void main()
{
	uint quadId = gl_InstanceID;
	uint vertId = gl_VertexID;
	vec3 qp = vec3(0.0f);
	qp = vertId == 1 || vertId == 4 ? vec3(1.0f, 0.0f, 0.0f) : qp;
	qp = vertId == 2 || vertId == 3 ? vec3(0.0f, 1.0f, 0.0f) : qp;
	qp = vertId == 5 ? vec3(1.0f, 1.0f, 0.0f) : qp;

	const float TerrainResolutionInv = 1.0 / float(gTerrainResolution);
	const float quadx = quadId / gTerrainResolution;
	const float quady = mod(quadId,gTerrainResolution);

	vec2 Uvs = (vec2(quadx, quady) + qp.xy) / gTerrainResolution;	
	vec4 WorldPos = SampleTerrain(quadx, quady, qp);


	vs_WorldPos = WorldPos;
	vs_Uvs.xy = Uvs;
	vs_position = gViewProjMat *WorldPos;

	vs_color = vec3(0.05 * (1.0 - gScreenshotCaptureActive));

	{
		const float offset = 5.0;
		vec4 WorldPos0_ = SampleTerrain(quadx + qp.x - offset, quady + qp.y, vec3(0.0f));
		vec4 WorldPos1_ = SampleTerrain(quadx + qp.x + offset, quady + qp.y, vec3(0.0f));
		vec4 WorldPos_0 = SampleTerrain(quadx + qp.x,     quady + qp.y - offset, vec3(0.0f));
		vec4 WorldPos_1 = SampleTerrain(quadx + qp.x,     quady + qp.y + offset, vec3(0.0f));
		vs_normal = cross(normalize(WorldPos1_.xyz - WorldPos0_.xyz), normalize(WorldPos_1.xyz - WorldPos_0.xyz));
		vs_normal = normalize(vs_normal);
	}


}
