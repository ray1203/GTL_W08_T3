// 파일명: DepthOnlyVS.hlsl

#include "ShaderRegisters.hlsl"

cbuffer LightViewProjCB : register(b1)
{
    matrix LightViewProj;
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

    // 2) 라이트 뷰·프로젝션 공간으로 변환
    output.Position = mul(worldPos, LightViewProj);

    return output;
}
