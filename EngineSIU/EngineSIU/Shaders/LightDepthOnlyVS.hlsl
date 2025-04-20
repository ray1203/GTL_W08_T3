// 파일명: DepthOnlyVS.hlsl

#include "ShaderRegisters.hlsl"

cbuffer LightViewProjCB : register(b1)
{
    row_major matrix LightViewProj;
};


// 정점 입력
struct VS_IN
{
    float3 Position : POSITION;
};

// 정점 출력
struct VS_OUT
{
    float4 Position : SV_POSITION;
};

VS_OUT mainVS(VS_IN input)
{
    VS_OUT output;

    float4 worldPos = mul(float4(input.Position, 1.0f), WorldMatrix);

    output.Position = mul(worldPos, LightViewProj);

    return output;
}
