cbuffer LetterboxParams : register(b0)
{
    float4 boxColor; // 예: float4(0,0,0,1)
    float targetAspectRatio; // 예: 16.0 / 9.0
    float windowAspectRatio; // 예: width / height
};

cbuffer FadeParams : register(b0)
{
    float4 fadeColor; // 예: float4(0,0,0,1)
    float FadeAmount;
    float3 FadePad;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// 풀스크린 쿼드 생성용 패스스루 정점 셰이더 (SV_VertexID 기반)
VS_OUTPUT ScreenQuadVS(uint vertexId : SV_VertexID)
{
    VS_OUTPUT output;

    float2 positions[6] =
    {
        float2(-1.0, -1.0),
        float2(-1.0, 1.0),
        float2(1.0, -1.0),
        float2(-1.0, 1.0),
        float2(1.0, 1.0),
        float2(1.0, -1.0)
    };
    float2 uvs[6] =
    {
        float2(0.0, 1.0),
        float2(0.0, 0.0),
        float2(1.0, 1.0),
        float2(0.0, 0.0),
        float2(1.0, 0.0),
        float2(1.0, 1.0)
    };

    output.pos = float4(positions[vertexId], 0.0, 1.0);
    output.uv = uvs[vertexId];
    return output;
}


float4 fadePS(VS_OUTPUT input) : SV_Target
{
    return FadeAmount * fadeColor;

}

// 픽셀 셰이더: uv로 레터박스 영역 판단 후 색상 출력
float4 letterBoxPS(VS_OUTPUT input) : SV_Target
{
    float2 uv = input.uv;
    bool isLetterbox = false;

    // 가로가 더 넓은 경우: 위/아래 검은 박스
    if (windowAspectRatio > targetAspectRatio)
    {
        float targetHeight = targetAspectRatio / windowAspectRatio;
        float margin = (1.0 - targetHeight) * 0.5;
        if (uv.y < margin || uv.y > 1.0 - margin)
            isLetterbox = true;
    }
    // 세로가 더 넓은 경우: 좌/우 검은 박스
    else
    {
        float targetWidth = windowAspectRatio / targetAspectRatio;
        float margin = (1.0 - targetWidth) * 0.5;
        if (uv.x < margin || uv.x > 1.0 - margin)
            isLetterbox = true;
    }

    if (isLetterbox)
        return boxColor;
    // 아니면 아무것도 안 칠함(원래 픽셀 유지). RTV에 덮어씌우는 상황이면 원본색을 샘플링해야 함.
    else
    {
        discard;
        return float4(0, 0, 0, 0);
    }
}
