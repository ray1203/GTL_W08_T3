
#include "ShaderRegisters.hlsl"

SamplerState DiffuseSampler : register(s0);
SamplerState NormalSampler : register(s1);


Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);


cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

cbuffer FlagConstants : register(b2)
{
    int IsLit;
    int bIsGammaCorrection;
    float2 flagPad0;
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
        FinalColor = float4(LitColor, 1);
#endif
    }
    else
    {
        FinalColor = float4(DiffuseColor, 1);
    }
    
    if (IsSelectedSubMesh)
    {
        FinalColor += float4(0.01, 0.01, 0.0, 1);
    }
    
    if(bIsGammaCorrection)
    {
        float gamma = 2.2;
        FinalColor = pow(FinalColor, 1.0 / gamma);
    }
    return FinalColor;
}
