Texture2D DepthSRV : register(t0);
SamplerState LinearSampler : register(s0);
 
cbuffer DepthMapData : register(b0)
{
    matrix ViewProj;
    float4 Params; //  x=Near, y=Far, z=1/Gamma, w=unused
};
 
struct VSInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

PSInput VS_Fullscreen(uint vid : SV_VertexID)
{
    float2 verts[3] = { float2(-1, -1), float2(-1, 3), float2(3, -1) };
    PSInput output;
    output.pos = float4(verts[vid], 0, 1);
    output.uv = verts[vid] * 0.5f + 0.5f;
    
    output.uv.y = 1.0f - output.uv.y;
    
    return output;
}

float4 PS_DepthVisualize(PSInput i) : SV_Target
{
    float ndc = DepthSRV.SampleLevel(LinearSampler, i.uv, 0);
    
    float nearZ = Params.x;
    float farZ = Params.y;
    float invG = Params.z;
    
    float A = farZ / (farZ - nearZ);
    float B = -farZ * nearZ / (farZ - nearZ);
    
    float viewZ = B / (ndc - A);
    
    float lin01 = saturate((viewZ - nearZ) / (farZ - nearZ));
    lin01 = pow(lin01, invG);
    
    return float4(lin01, lin01, lin01, 1.0f);
}
