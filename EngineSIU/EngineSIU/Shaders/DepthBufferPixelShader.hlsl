//DepthBufferPixelShader

#define CAMERA_NEAR 0.1f
#define CAMERA_FAR  100.0f

Texture2D SceneDepthTexture : register(t0);
SamplerState DepthSampler : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

float4 mainPS(PS_INPUT Input) : SV_TARGET
{
  
    return float4(1,0,0,1);
    float DepthRaw = SceneDepthTexture.Sample(DepthSampler, Input.UV).r;

}
