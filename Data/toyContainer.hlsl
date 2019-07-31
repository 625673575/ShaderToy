typedef float2 vec2;
typedef float3 vec3;
typedef float4 vec4;
typedef float2x2 mat2;
typedef float3x3 mat3;
typedef float4x4 mat4;

#define mix lerp
#define fract frac
#define mod(x, y) (x - y * floor(x / y))

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


#include "toy.hlsl"

float4 main(in float2 texC : TEXCOORD) : SV_TARGET
{
    return mainImage(texC * iResolution);
}
