#version 330 core

out vec4 fragColor;

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texcoord;

struct Light
{
    float intensity;
    vec3 color;
    vec3 points[4];
    bool twoSided;
};
uniform Light areaLight;
uniform vec3 areaLightTranslate;

struct Material
{
    sampler2D diffuse;
    vec4 albedoRoughness; // (x,y,z) = color, w = roughness
};
uniform Material material;

uniform vec3 viewPosition;
uniform sampler2D LTC1; // for inverse M
uniform sampler2D LTC2; // GGX norm, fresnel, 0(unused), sphere
uniform sampler2DArray u_FilteredLightTexture;


const float LUT_SIZE  = 64.0; // ltc_texture size
const float LUT_SCALE = (LUT_SIZE - 1.0)/LUT_SIZE;
const float LUT_BIAS  = 0.5/LUT_SIZE;


// Vector form without project to the plane (dot with the normal)
// Use for proxy sphere clipping
vec3 IntegrateEdgeVec(vec3 v1, vec3 v2)
{
    // Using built-in acos() function will result flaws
    // Using fitting result for calculating acos()
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206*y)*y;
    float b = 3.4175940 + (4.1616724 + y)*y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5*inversesqrt(max(1.0 - x*x, 1e-7)) - v;

    return cross(v1, v2)*theta_sintheta;
}

float IntegrateEdge(vec3 v1, vec3 v2)
{
    return IntegrateEdgeVec(v1, v2).z;
}

struct SRay
{
    vec3 m_Origin;
    vec3 m_Dir;
};

vec3 integrateEdge(vec3 vVertex1, vec3 vVertex2)
{
	float CosTheta = dot(vVertex1, vVertex2);
	float Theta = acos(CosTheta);
	return cross(vVertex1, vVertex2) * ((Theta > 0.001) ? Theta / sin(Theta) : 1.0);
}

bool rayPlaneIntersect(SRay vRay, vec4 vPlane, out float voDistanceFromRayOrigin2Plane)
{
    voDistanceFromRayOrigin2Plane = -dot(vPlane, vec4(vRay.m_Origin, 1.0)) / dot(vPlane.xyz, vRay.m_Dir);
    return voDistanceFromRayOrigin2Plane > 0.0;
}


vec3 fecthFilteredLightTexture(vec3 vPolygonalLightVertexPos[4], vec3 vLooupVector)
{
	vec3 V1 = vPolygonalLightVertexPos[1] - vPolygonalLightVertexPos[0];
	vec3 V2 = vPolygonalLightVertexPos[3] - vPolygonalLightVertexPos[0];
	vec3 PlaneOrtho = cross(V1, V2);
	float PlaneAreaSquared = dot(PlaneOrtho, PlaneOrtho);
	
	SRay Ray;
	Ray.m_Origin = vec3(0);
	Ray.m_Dir = vLooupVector;
	vec4 Plane = vec4(PlaneOrtho, -dot(PlaneOrtho, vPolygonalLightVertexPos[0]));
	float Distance2Plane;
	rayPlaneIntersect(Ray, Plane, Distance2Plane);
	
	vec3 P = Distance2Plane * Ray.m_Dir - vPolygonalLightVertexPos[0];

	float Dot_V1_V2 = dot(V1, V2);
	float Inv_Dot_V1_V1 = 1.0 / dot(V1, V1);
	vec3  V2_ = V2 - V1 * Dot_V1_V2 * Inv_Dot_V1_V1;
	vec2  UV;
	UV.y = dot(V2_, P) / dot(V2_, V2_);
	UV.x = dot(V1, P) * Inv_Dot_V1_V1 - Dot_V1_V2 * Inv_Dot_V1_V1 * UV.y;
	UV = vec2(1 - UV.y, 1 - UV.x);
	
	float Distance = abs(Distance2Plane) / pow(PlaneAreaSquared, 0.25);
	
	float Lod = log(2048.0 * Distance) / log(3.0);

	float LodA = floor(Lod);
	float LodB = ceil(Lod);
	float t = Lod - LodA;
	vec3 ColorA = texture(u_FilteredLightTexture, vec3(UV, LodA)).rgb;
	vec3 ColorB = texture(u_FilteredLightTexture, vec3(UV, LodB)).rgb;
	return mix(ColorA, ColorB, t);
}




// P is fragPos in world space (LTC distribution)
vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided)
{
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N * dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = Minv * transpose(mat3(T1, T2, N));

    // polygon (allocate 4 vertices for clipping)
    vec3 L[4];
    // transform polygon from LTC back to origin Do (cosine weighted)
    L[0] = Minv * (points[0] - P);
    L[1] = Minv * (points[1] - P);
    L[2] = Minv * (points[2] - P);
    L[3] = Minv * (points[3] - P);

    // use tabulated horizon-clipped sphere
    // check if the shading point is behind the light
    vec3 dir = points[0] - P; // LTC space
    vec3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
    bool behind = (dot(dir, lightNormal) < 0.0);

    // cos weighted space
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    // integrate
    vec3 vsum = vec3(0.0);
    vsum += IntegrateEdgeVec(L[0], L[1]);
    vsum += IntegrateEdgeVec(L[1], L[2]);
    vsum += IntegrateEdgeVec(L[2], L[3]);
    vsum += IntegrateEdgeVec(L[3], L[0]);

    // form factor of the polygon in direction vsum
    float len = length(vsum);

    float z = vsum.z/len;
    if (behind)
        z = -z;

    vec2 uv = vec2(z*0.5f + 0.5f, len); // range [0, 1]
    uv = uv*LUT_SCALE + LUT_BIAS;

    // Fetch the form factor for horizon clipping
    float scale = texture(LTC2, uv).w;

    float sum = len*scale;
    if (!behind && !twoSided)
        sum = 0.0;

    vec3 LookupVector = normalize(vsum);
	vec3 TextureLight = fecthFilteredLightTexture(L, LookupVector);

    // Outgoing radiance (solid angle) for the entire polygon
    //vec3 Lo_i = vec3(sum, sum, sum);
    //return Lo_i;
    return vec3(sum) * TextureLight;
}

void clipPolygonalLightByZPlane(inout vec3 vioPolygonalLightVertexPos[5], out int voPolygonalLightVertexNumAfterClipping)
{
	int Flag4HowManyVertexAboveZPlane = 0;
	if(vioPolygonalLightVertexPos[0].z > 0.0) Flag4HowManyVertexAboveZPlane += 1;
	if(vioPolygonalLightVertexPos[1].z > 0.0) Flag4HowManyVertexAboveZPlane += 2;
	if(vioPolygonalLightVertexPos[2].z > 0.0) Flag4HowManyVertexAboveZPlane += 4;
	if(vioPolygonalLightVertexPos[3].z > 0.0) Flag4HowManyVertexAboveZPlane += 8;

	voPolygonalLightVertexNumAfterClipping = 0;
	if(Flag4HowManyVertexAboveZPlane == 0)
	{
	}
	else if(Flag4HowManyVertexAboveZPlane == 1)
	{
		voPolygonalLightVertexNumAfterClipping = 3;
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1];
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3];
	}
	else if(Flag4HowManyVertexAboveZPlane == 2)
	{
		voPolygonalLightVertexNumAfterClipping = 3;
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0];
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2];
	}
	else if(Flag4HowManyVertexAboveZPlane == 3)
	{
		voPolygonalLightVertexNumAfterClipping = 4;
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2];
		vioPolygonalLightVertexPos[3] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3];
	}
	else if(Flag4HowManyVertexAboveZPlane == 4)
	{
		voPolygonalLightVertexNumAfterClipping = 3;
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3];
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1];
	}
	else if(Flag4HowManyVertexAboveZPlane == 5)
	{
		voPolygonalLightVertexNumAfterClipping = 0;
	}
	else if(Flag4HowManyVertexAboveZPlane == 6)
	{
		voPolygonalLightVertexNumAfterClipping = 4;
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0];
		vioPolygonalLightVertexPos[3] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3];
	}
	else if(Flag4HowManyVertexAboveZPlane == 7)
	{
		voPolygonalLightVertexNumAfterClipping = 5;
		vioPolygonalLightVertexPos[4] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3];
		vioPolygonalLightVertexPos[3] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3];
	}
	else if(Flag4HowManyVertexAboveZPlane == 8)
	{
		voPolygonalLightVertexNumAfterClipping = 3;
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0];
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2];
		vioPolygonalLightVertexPos[2] = vioPolygonalLightVertexPos[3];
	}
	else if(Flag4HowManyVertexAboveZPlane == 9)
	{
		voPolygonalLightVertexNumAfterClipping = 4;
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1];
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2];
	}
	else if(Flag4HowManyVertexAboveZPlane == 10)
	{
		voPolygonalLightVertexNumAfterClipping = 0;
	}
	else if(Flag4HowManyVertexAboveZPlane == 11)
	{
		voPolygonalLightVertexNumAfterClipping = 5;
		vioPolygonalLightVertexPos[4] = vioPolygonalLightVertexPos[3];
		vioPolygonalLightVertexPos[3] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2];
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2];
	}
	else if(Flag4HowManyVertexAboveZPlane == 12)
	{
		voPolygonalLightVertexNumAfterClipping = 4;
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1];
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0];
	}
	else if(Flag4HowManyVertexAboveZPlane == 13)
	{
		voPolygonalLightVertexNumAfterClipping = 5;
		vioPolygonalLightVertexPos[4] = vioPolygonalLightVertexPos[3];
		vioPolygonalLightVertexPos[3] = vioPolygonalLightVertexPos[2];
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1];
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1];
	}
	else if(Flag4HowManyVertexAboveZPlane == 14)
	{
		voPolygonalLightVertexNumAfterClipping = 5;
		vioPolygonalLightVertexPos[4] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0];
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0];
	}
	else if(Flag4HowManyVertexAboveZPlane == 15)
	{
		voPolygonalLightVertexNumAfterClipping = 4;
	}

	if(voPolygonalLightVertexNumAfterClipping == 3)
		vioPolygonalLightVertexPos[3] = vioPolygonalLightVertexPos[0];
	if(voPolygonalLightVertexNumAfterClipping == 4)
		vioPolygonalLightVertexPos[4] = vioPolygonalLightVertexPos[0];
}

vec3 integrateLTC(vec3 vNormal, vec3 vViewDir, vec3 vFragPos, mat3 vLTCMatrix, vec3 vPolygonalLightVertexPos[4], bool vTwoSided)
{	
	//着色点上的切线空间正交基
	vec3 Tangent = normalize(vViewDir - vNormal * dot(vViewDir, vNormal));
	vec3 Bitangent = cross(vNormal, Tangent);

	//将变换矩阵转换到切线空间
	vLTCMatrix = vLTCMatrix * transpose(mat3(Tangent, Bitangent, vNormal));
	//多边形顶点(因为被平面z=0裁剪以后，四边形可能变成5个顶点)
	vec3 PolygonalLightVertexPosInTangentSpace[5], PolygonalLightVertexPosBeforeClipping[4];
	PolygonalLightVertexPosBeforeClipping[0] = PolygonalLightVertexPosInTangentSpace[0] = vLTCMatrix * (vPolygonalLightVertexPos[0] - vFragPos);
	PolygonalLightVertexPosBeforeClipping[1] = PolygonalLightVertexPosInTangentSpace[1] = vLTCMatrix * (vPolygonalLightVertexPos[1] - vFragPos);
	PolygonalLightVertexPosBeforeClipping[2] = PolygonalLightVertexPosInTangentSpace[2] = vLTCMatrix * (vPolygonalLightVertexPos[2] - vFragPos);
	PolygonalLightVertexPosBeforeClipping[3] = PolygonalLightVertexPosInTangentSpace[3] = vLTCMatrix * (vPolygonalLightVertexPos[3] - vFragPos);
	PolygonalLightVertexPosInTangentSpace[4] = PolygonalLightVertexPosInTangentSpace[3];

	int PolygonalLightVertexNumAfterClipping;
	clipPolygonalLightByZPlane(PolygonalLightVertexPosInTangentSpace, PolygonalLightVertexNumAfterClipping);
	
	if(PolygonalLightVertexNumAfterClipping == 0)
		return vec3(0);
	
	//把裁剪后的多边形投影到球面上（也就是对每个顶点坐标向量归一化）
	PolygonalLightVertexPosInTangentSpace[0] = normalize(PolygonalLightVertexPosInTangentSpace[0]);
	PolygonalLightVertexPosInTangentSpace[1] = normalize(PolygonalLightVertexPosInTangentSpace[1]);
	PolygonalLightVertexPosInTangentSpace[2] = normalize(PolygonalLightVertexPosInTangentSpace[2]);
	PolygonalLightVertexPosInTangentSpace[3] = normalize(PolygonalLightVertexPosInTangentSpace[3]);
	PolygonalLightVertexPosInTangentSpace[4] = normalize(PolygonalLightVertexPosInTangentSpace[4]);

	//用累加边的公式来求解积分
	vec3 VSum = vec3(0.0);
	VSum += integrateEdge(PolygonalLightVertexPosInTangentSpace[0], PolygonalLightVertexPosInTangentSpace[1]);
	VSum += integrateEdge(PolygonalLightVertexPosInTangentSpace[1], PolygonalLightVertexPosInTangentSpace[2]);
	VSum += integrateEdge(PolygonalLightVertexPosInTangentSpace[2], PolygonalLightVertexPosInTangentSpace[3]);
	if(PolygonalLightVertexNumAfterClipping >= 4)
		VSum += integrateEdge(PolygonalLightVertexPosInTangentSpace[3], PolygonalLightVertexPosInTangentSpace[4]);
	if(PolygonalLightVertexNumAfterClipping == 5)
		VSum += integrateEdge(PolygonalLightVertexPosInTangentSpace[4], PolygonalLightVertexPosInTangentSpace[0]);

	float Sum = vTwoSided ? abs(VSum.z) : max(VSum.z, 0.0);

	vec3 LookupVector = normalize(VSum);
	vec3 TextureLight = fecthFilteredLightTexture(PolygonalLightVertexPosBeforeClipping, LookupVector);

	return vec3(Sum) * TextureLight;
}


// PBR-maps for roughness (and metallic) are usually stored in non-linear
// color space (sRGB), so we use these functions to convert into linear RGB.
vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float gamma = 2.2;
vec3 ToLinear(vec3 v) { return PowVec3(v, gamma); }
vec3 ToSRGB(vec3 v)   { return PowVec3(v, 1.0/gamma); }


void main()
{
    // gamma correction
    vec3 mDiffuse = texture(material.diffuse, texcoord).xyz;// * vec3(0.7f, 0.8f, 0.96f);
    vec3 mSpecular = ToLinear(vec3(0.23f, 0.23f, 0.23f)); // mDiffuse

    vec3 result = vec3(0.0f);

    vec3 N = normalize(worldNormal);
    vec3 V = normalize(viewPosition - worldPosition);
    vec3 P = worldPosition;
    float dotNV = clamp(dot(N, V), 0.0f, 1.0f);

    // use roughness and sqrt(1-cos_theta) to sample M_texture
    vec2 uv = vec2(material.albedoRoughness.w, sqrt(1.0f - dotNV));
    uv = uv*LUT_SCALE + LUT_BIAS;
	//uv = vec2(uv.x, 1.0-uv.y);

    // get 4 parameters for inverse_M
    vec4 t1 = texture(LTC1, uv);

    // Get 2 parameters for Fresnel calculation
    vec4 t2 = texture(LTC2, uv);

    mat3 Minv = mat3(
        vec3(t1.x, 0, t1.y),
        vec3(  0,  1,    0),
        vec3(t1.z, 0, t1.w)
    );

    // translate light source for testing
    vec3 translatedPoints[4];
    translatedPoints[0] = areaLight.points[0] + areaLightTranslate;
    translatedPoints[1] = areaLight.points[1] + areaLightTranslate;
    translatedPoints[2] = areaLight.points[2] + areaLightTranslate;
    translatedPoints[3] = areaLight.points[3] + areaLightTranslate;

    // Evaluate LTC shading
    //vec3 diffuse = LTC_Evaluate(N, V, P, mat3(1), translatedPoints, areaLight.twoSided);
    //vec3 specular = LTC_Evaluate(N, V, P, Minv, translatedPoints, areaLight.twoSided);

	vec3 diffuse = integrateLTC(N, V, P, mat3(1), translatedPoints, areaLight.twoSided);
	vec3 specular = integrateLTC(N, V, P, Minv, translatedPoints, areaLight.twoSided);
    // GGX BRDF shadowing and Fresnel
    // t2.x: shadowedF90 (F90 normally it should be 1.0)
    // t2.y: Smith function for Geometric Attenuation Term, it is dot(V or L, H).
    specular *= mSpecular*t2.x + (1.0f - mSpecular) * t2.y;
	mDiffuse = vec3(1.0f, 1.0f, 1.0f);
    result = areaLight.color * areaLight.intensity * (specular + mDiffuse * diffuse);

    fragColor = vec4(ToSRGB(result), 1.0f);
}