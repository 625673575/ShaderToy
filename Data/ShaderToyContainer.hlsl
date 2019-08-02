typedef float2 vec2;
typedef float3 vec3;
typedef float4 vec4;
typedef float2x2 mat2;
typedef float3x3 mat3;
typedef float4x4 mat4;
typedef int2 ivec2;
typedef int3 ivec3;
typedef int4 ivec4;
typedef bool2 bvec2;
typedef bool3 bvec3;
typedef bool4 bvec4;
typedef Texture1D sampler1D;
typedef Texture2D sampler2D;
typedef Texture3D sampler3D;
typedef TextureCube samplerCube;

#define mix lerp
#define fract frac
#define mod(x, y) (x - (y * floor((x)/ (y))))
#define atan(y, x) (atan2(x, y))
#define dFdx(x) ddx(x)
#define dFdxCoarse(x) ddx_coarse(x)
#define dFdxFine(x) ddx_fine(x)
#define dFdy(x) ddy(x)
#define dFdyCoarse(x) ddy_coarse
#define dFdyFine(x) ddy_fine(x)
//return a*b+c
#define fma(a,b,c) mad(a,b,c)
 
#define texture(tex,uv) (tex.Sample(gSampler, (uv)))
#define texelFetch(tex,coord,lod) (tex.Load(coord,lod))
#define texelFetchOffset(tex,coord,lod,offset) (tex.Load(coord,lod,offset))
#define textureOffset(tex,coord,offset) (tex.SampleBias(gSampler,coord,coord,offset))

cbuffer ToyCB : register(b0)
{
    float2 iResolution;
    float iTime;
    float iTimeDelta;
    uint iFrame;
    float4 iMouse;
    Texture2D iChannel0;
    Texture2D iChannel1;
    Texture2D iChannel2;
    Texture2D iChannel3;
    SamplerState gSampler;
};


float4 main(in float2 texC : TEXCOORD) : SV_TARGET
{
    return mainImage(texC * iResolution);
}
