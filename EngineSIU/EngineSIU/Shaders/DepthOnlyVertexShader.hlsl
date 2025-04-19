// DepthOnlyVertexShader.hlsl

cbuffer FShadowViewProj : register(b0)
{
    row_major matrix ViewProj;
};

cbuffer FShadowWorld : register(b1)
{
    row_major matrix World;
};

struct VS_INPUT
{
    float3 Position : POSITION;
};

struct VS_OUTPUT
{
    float4 SV_Position : SV_POSITION;
};

VS_OUTPUT mainVS(VS_INPUT In)
{
    VS_OUTPUT Out;
    // 월드 변환
    float4 worldPos = mul(float4(In.Position, 1.0f), World);
    // 뷰프로젝션 변환
    Out.SV_Position = mul(worldPos, ViewProj);
    return Out;
}
