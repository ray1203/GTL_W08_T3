
#include "ShaderRegisters.hlsl"

SamplerState DiffuseSampler : register(s0);
SamplerState NormalSampler : register(s1);
SamplerComparisonState ShadowSampler : register(s5);


Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D<float> ShadowTexture : register(t5);


cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

cbuffer FlagConstants : register(b2)
{
    int IsLit;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b3)
{
    int IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b4)
{
    float2 UVOffset;
    float2 TexturePad0;
}

#include "Light.hlsl"

cbuffer LightViewProjCB : register(b5)
{
    row_major matrix LightViewProj;
};

float CalcShadowHW(float4 worldPos)
{
    // 라이트 클립 공간으로 변환
    float4 lightSpacePos = mul(worldPos, LightViewProj);

    float4 lsPos = mul(worldPos, LightViewProj);
    float2 uv = lsPos.xy / lsPos.w * 0.5f + 0.5f;
    // 0  ~ 1
    uv.y = 1 - uv.y;
    float depth = lsPos.z / lsPos.w;
    
    // 레벨 0 비교 샘플링
    return ShadowTexture.SampleCmpLevelZero(ShadowSampler, uv, depth);
}


float4 mainPS(PS_INPUT_StaticMesh Input) : SV_Target
{
    float4 FinalColor = float4(0.f, 0.f, 0.f, 1.f);

    // Diffuse
    float3 DiffuseColor = Material.DiffuseColor;
    if (Material.TextureFlag & (1 << 1))
    {
        DiffuseColor = DiffuseTexture.Sample(DiffuseSampler, Input.UV).rgb;
        DiffuseColor = SRGBToLinear(DiffuseColor);
    }

    // Normal
    float3 WorldNormal = Input.WorldNormal;
    if (Material.TextureFlag & (1 << 2))
    {
        float3 Normal = NormalTexture.Sample(NormalSampler, Input.UV).rgb;
        Normal = normalize(2.f * Normal - 1.f);
        WorldNormal = normalize(mul(mul(Normal, Input.TBN), (float3x3) InverseTransposedWorld));
    }
    
    // Lighting
    if (IsLit)
    {
#ifdef LIGHTING_MODEL_GOURAUD
        FinalColor = float4(Input.Color.rgb * DiffuseColor, 1.0);
#else
        float3 LitColor = Lighting(Input.WorldPosition, WorldNormal, Input.WorldViewPosition, DiffuseColor).rgb;
        float ShadowColor = CalcShadowHW(float4(Input.WorldPosition, 1));
        FinalColor = float4(LitColor * ShadowColor, 1);
#endif
    }
    else
    {
        FinalColor = float4(DiffuseColor, 1);
    }
    
    if (bIsSelected)
    {
        FinalColor += float4(0.01, 0.01, 0.0, 1);
    }
    
    return FinalColor;
}
