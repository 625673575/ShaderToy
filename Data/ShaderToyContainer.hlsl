typedef float2 vec2;
typedef float3 vec3;
typedef float4 vec4;
typedef float2x2 mat2;
typedef float3x3 mat3;
typedef float4x4 mat4;

#define mix lerp
#define fract frac
#define mod(x, y) (x - y * floor(x / y))
#define atan(y, x) atan2(x, y)
#define texture(tex,uv) tex.Sample(gSampler, uv);

cbuffer ToyCB : register(b0)
{
    float2 iResolution;
    float iTime;
    float iTimeDelta;
    float iFrame;
    float iChannelTime[4];
    float4 iMouse;
    float4 iDate;
    float iSampleRate;
    float3 iChannelResolution[4];
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
