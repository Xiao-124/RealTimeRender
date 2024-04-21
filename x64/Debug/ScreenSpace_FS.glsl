#version 430 core

uniform sampler2D  PositionTexture;
uniform sampler2D  NormalTexture;
uniform sampler2D  ColorTexture;
uniform sampler2D  DownscaledDepth;



layout(std140, binding = 0) uniform u_Matrices4ProjectionWorld
{
    mat4 u_ProjectionMatrix;
    mat4 u_ViewMatrix;
};

uniform float u_Near = 0.1;
uniform float u_Far = 1000.0f;
uniform int maxLevel;

in vec2 v2f_TexCoords;
out vec4 FragColor;


#define INIT_STEP 0.4
#define MAX_STEPS 200
#define EPS 1e-2
#define THRES 0.1
#define INV_PI 0.31830988618
#define MAX_THICKNESS 0.0017
float distanceSquared(vec2 A, vec2 B)
{
    A -= B;
    return dot(A, A);
}
float LinearizeDepth(float vDepth)
{
    float z = vDepth * 2.0 - 1.0;
    return (2.0 * u_Near * u_Far) / (u_Far + u_Near - z * (u_Far - u_Near));
}

vec3 GetScreenCoord(vec3  Point) {

    vec4 positionInScreen = u_ProjectionMatrix * vec4(Point, 1.0);
    positionInScreen.xyzw /= positionInScreen.w;
    positionInScreen.xyz = positionInScreen.xyz * 0.5 + 0.5;
    return positionInScreen.xyz;

}
vec3 RayMarch(vec3 ori, vec3 dir) 
{
    vec3 Point = ori;
    for (int i = 0; i < MAX_STEPS; i++) 
    {
        Point += dir * INIT_STEP;
        vec4 positionInScreen = u_ProjectionMatrix * vec4(Point, 1.0);
        positionInScreen.xyzw /= positionInScreen.w;
        positionInScreen.xyz = positionInScreen.xyz * 0.5 + 0.5;
        if (positionInScreen.x < 0 || positionInScreen.x>1 || positionInScreen.y < 0 || positionInScreen.y>1) 
        {
            return vec3(0.0);
        }
        float depth = texture2D(PositionTexture, positionInScreen.xy).a;
        if (depth < -Point.z + 0.025 || i == MAX_STEPS - 1) 
        {
            return texture2D(ColorTexture, positionInScreen.st).rgb;
        }

    }
    return vec3(0.0);
}


struct Result
{
    bool IsHit;
    vec2 UV;
    vec3 Position;

    int IterationCount;
};


bool Query(vec2 z, vec2 uv)
{
    float depths = -LinearizeDepth(texture(PositionTexture, uv / vec2(1024.0, 1024.0)).a);
    //    return z.y < depths;
    return z.y < depths && z.x > depths;
}

Result RayMarch2D(vec3 ori, vec3 dir) 
{
    Result result;
    vec3 start = ori;
    vec3 end = start + 1000 * dir;

    vec4 H0 = u_ProjectionMatrix * vec4(ori, 1.0);
    vec4 H1 = u_ProjectionMatrix * vec4(end, 1.0);

    float k0 = 1.0 / H0.w;
    float k1 = 1.0 / H1.w;

    vec3 Q0 = start * k0;
    vec3 Q1 = end * k1;


    vec2 P0 = H0.xy * k0;
    vec2 P1 = H1.xy * k1;
    vec2 size = vec2(1024.0, 1024.0);
    P0 = (P0 + 1) / 2 * size;
    P1 = (P1 + 1) / 2 * size;


    P1 += vec2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);

    vec2 Delta = P1 - P0;
    bool Permute = false;
    if (abs(Delta.x) < abs(Delta.y))
    {
        Permute = true;
        Delta = Delta.yx; P0 = P0.yx; P1 = P1.yx;
    }
    float stepDir = sign(Delta.x);
    float invDx = stepDir / Delta.x;

    vec2 dP = vec2(stepDir, Delta.y * invDx);
    vec3 dQ = (Q1 - Q0) * invDx;
    float dk = (k1 - k0) * invDx;


    P0 += dP; Q0 += dQ; k0 += dk;

    int Step = 0;
    int MaxStep = 1000;
    float k = k0;
    float EndX = P1.x * stepDir;
    vec3 Q = Q0;
    float prevZMaxEstimate = start.z;

    for (vec2 P = P0; Step < MaxStep; Step++, P += dP, Q.z += dQ.z, k += dk)
    {
        result.UV = Permute ? P.yx : P;
        vec2 Depths;
        Depths.x = prevZMaxEstimate;
        Depths.y = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
        prevZMaxEstimate = Depths.y;
        if (Depths.x < Depths.y)
            Depths.xy = Depths.yx;
        if (result.UV.x > 1024.0 || result.UV.x < 0 || result.UV.y > 1024.0 || result.UV.y < 0)
            break;
        result.IsHit = Query(Depths, result.UV);
        if (result.IsHit)
            break;
    }

    return result;

}
vec3 trace_ray(vec3 ray_start, vec3 ray_dir)
{

    if (ray_dir.z < 0.0) 
    {
        return vec3(0);
    }

    ray_dir = normalize(ray_dir);
    ivec2 work_size = ivec2(1024, 1024);

    const int loop_max = 200;
    int mipmap = 0;
    int max_iter = loop_max;

    vec3 pos = ray_start;

    // Move pos by a small bias
    pos += ray_dir * 0.008;

    float hit_bias = 0.0017;

    while (mipmap > -1 && max_iter-- > 0)
    {

        // Check if we are out of screen bounds, if so, return
        if (pos.x < 0.0 || pos.y < 0.0 || pos.x > 1.0 || pos.y > 1.0 || pos.z < 0.0 || pos.z > 1.0)
        {
            return vec3(0, 0, 0);
        }

        // Fetch the current minimum cell plane height
        float cell_z = textureLod(DownscaledDepth, pos.xy, mipmap).x;

        // Compute the fractional part of the coordinate (scaled by the working size)
        // so the values will be between 0.0 and 1.0
        vec2 fract_coord = mod(pos.xy * work_size, 1.0);

        // Modify fract coord based on which direction we are stepping in.
        // Fract coord now contains the percentage how far we moved already in
        // the current cell in each direction.  
        fract_coord.x = ray_dir.x > 0.0 ? fract_coord.x : 1.0 - fract_coord.x;
        fract_coord.y = ray_dir.y > 0.0 ? fract_coord.y : 1.0 - fract_coord.y;

        // Compute maximum k and minimum k for which the ray would still be
        // inside of the cell.
        vec2 max_k_v = (1.0 / abs(ray_dir.xy)) / work_size.xy;
        vec2 min_k_v = -max_k_v * fract_coord.xy;

        // Scale the maximum k by the percentage we already processed in the current cell,
        // since e.g. if we already moved 50%, we can only move another 50%.
        max_k_v *= 1.0 - fract_coord.xy;

        // The maximum k is the minimum of the both sub-k's since if one component-maximum
        // is reached, the ray is out of the cell
        float max_k = min(max_k_v.x, max_k_v.y);

        // Same applies to the min_k, but because min_k is negative we have to use max()
        float min_k = max(min_k_v.x, min_k_v.y);

        // Check if the ray intersects with the cell plane. We have the following
        // equation: 
        // pos.z + k * ray_dir.z = cell.z
        // So k is:
        float k = (cell_z - pos.z) / ray_dir.z;

        // Optional: Abort when ray didn't exactly intersect:
        // if (k < min_k && mipmap <= 0) {
        //     return vec3(0);
        // } 

        // Check if we intersected the cell
        if (k < max_k + hit_bias)
        {
            // Clamp k
            k = max(min_k, k);

            if (mipmap < 1) {
                pos += k * ray_dir;
                return pos;
            }

            // If we hit anything at a higher mipmap, step up to a higher detailed
            // mipmap:
            mipmap -= 2;
            work_size *= 4;
        }
        else {

            // If we hit nothing, move to the next cell, with a small bias
            pos += max_k * ray_dir * 1.04;
        }

        mipmap += 1;
        work_size /= 2;
    }

    return vec3(0);
}
vec2 getCellCount(int level) {
    return textureSize(DownscaledDepth, level);
}

vec2 getCell(vec2 pos, vec2 startCellCount) 
{

    return vec2(floor(pos * startCellCount));
}

vec3 intersectDepthPlane(vec3 o, vec3 d, float t) 
{
    return o + d * t;
}

vec3 intersectCellBoundary(vec3 o, vec3  d, vec2 rayCell, vec2 cell_count, vec2 crossStep, vec2 crossOffset) 
{

    vec2 nextPos = rayCell + crossStep;
    nextPos = nextPos / cell_count;
    nextPos = nextPos + crossOffset;

    vec2 dis = nextPos - o.xy;

    vec2 delta = dis / d.xy;

    float t = min(delta.x, delta.y);

    return intersectDepthPlane(o, d, t);


}
float getMinimumDepthPlane(vec2 pos, int level) 
{

    return textureLod(DownscaledDepth, pos, level).r;
}

bool crossedCellBoundary(vec2 oldCellIdx, vec2 newCellIdx) 
{
    return (oldCellIdx.x != newCellIdx.x) || (oldCellIdx.y != newCellIdx.y);
}

vec3 FindIntersection(vec3 start, vec3 rayDir, float maxTraceDistance, vec3 hitPos) {

    vec2 crossStep = vec2(rayDir.x >= 0 ? 1 : -1, rayDir.y >= 0 ? 1 : -1);
    vec2 crossOffset = crossStep / vec2(1024.0, 1024.0) / 128;
    crossStep = clamp(crossStep, 0.0, 1.0);

    vec3 ray = start;
    float minZ = ray.z;
    float maxZ = ray.z + rayDir.z * maxTraceDistance;
    float deltaZ = (maxZ - minZ);

    vec3 o = ray;
    vec3 d = rayDir * maxTraceDistance;

    int startLevel = 2;
    int stopLevel = 0;
    vec2 startCellCount = getCellCount(startLevel);


    vec2 rayCell = getCell(ray.xy, startCellCount);
    ray = intersectCellBoundary(o, d, rayCell, startCellCount, crossStep, crossOffset * 64);

    int level = startLevel;
    int iter = 0;
    bool isBackwardRay = rayDir.z < 0;

    float Dir = isBackwardRay ? -1 : 1;

    while (level >= stopLevel && ray.z * Dir <= maxZ * Dir && iter < 100) 
    {
        vec2 cellCount = getCellCount(level);
        vec2 oldCellIdx = getCell(ray.xy, cellCount);

        float cell_minZ = getMinimumDepthPlane(ray.xy, level);

        vec3 tmpRay = ((cell_minZ > ray.z) && !isBackwardRay) ? intersectDepthPlane(o, d, (cell_minZ - minZ) / deltaZ) : ray;

        vec2 newCellIdx = getCell(tmpRay.xy, cellCount);

        float thickness = level == 0 ? (ray.z - cell_minZ) : 0;
        bool crossed = (isBackwardRay && (cell_minZ > ray.z)) || (thickness > MAX_THICKNESS) || crossedCellBoundary(oldCellIdx, newCellIdx);

        ray = crossed ? intersectCellBoundary(o, d, oldCellIdx, cellCount, crossStep, crossOffset) : tmpRay;
        level = crossed ? min(maxLevel, level + 1) : level - 1;
        ++iter;

    }
    bool intersected = (level < stopLevel);
    hitPos = ray;

    return intersected ? texture(ColorTexture, hitPos.xy).rgb : vec3(0.0);


}
void main() {

    vec3 positionInView = texture(PositionTexture, v2f_TexCoords).rgb;
    vec3 normalInView = texture(NormalTexture, v2f_TexCoords).rgb;
    vec3 relfectDir = normalize(reflect(positionInView, normalInView));


    vec3 endPosInView = positionInView + relfectDir * 1000;

    vec3 start = GetScreenCoord(positionInView);
    vec3 end = GetScreenCoord(endPosInView);
    vec3 rayDir = normalize(end - start);

    float maxTraceX = rayDir.x >= 0 ? (1 - start.x) / rayDir.x : -start.x / rayDir.x;
    float maxTraceY = rayDir.y >= 0 ? (1 - start.y) / rayDir.y : -start.y / rayDir.y;
    float maxTraceZ = rayDir.z >= 0 ? (1 - start.z) / rayDir.z : -start.z / rayDir.z;
    float maxTraceDistance = min(maxTraceX, min(maxTraceY, maxTraceZ));

    vec3 hitPos = vec3(0.0);

    vec3 L_indir = FindIntersection(start, rayDir, maxTraceDistance, hitPos) * texture(ColorTexture, v2f_TexCoords).rgb * INV_PI;


    //vec4 LightPosInView = view * model * vec4(LightPos, 1.0);
    //vec3 wi = normalize(vec3(LightPosInView.xyz) - positionInView);
    //
    //    
    //  
    vec3  hitpos = trace_ray(start, rayDir);
    // 
    //   
    //
    //
    //    vec3 L_indir;
    //    if(hitpos != vec3(0.0)){
    //        L_indir = texture2D(ColorTexture,hitpos.st).rgb;
    //    }
    //	
    //    //L_indir = texture2DLod(DownscaledDepth,texCoords,5).rgb;
    //	//L_indir = RayMarch(positionInView,relfectDir)*texture(ColorTexture,texCoords).rgb*INV_PI;
    ////    Result result = RayMarch2D(positionInView,relfectDir);
    ////    if(result.IsHit){
    ////        L_indir = texture2D(ColorTexture,result.UV / vec2(1024.0,1024.0)).rgb;
    ////    
    ////    }
    //    
    //
    vec3 L = texture(ColorTexture, v2f_TexCoords).rgb ;

    L += L_indir*5;
    vec3 color = pow(clamp(L, vec3(0.0), vec3(1.0)), vec3(1.0 / 2.2));
    FragColor = vec4(L, 1.0);
    //



}