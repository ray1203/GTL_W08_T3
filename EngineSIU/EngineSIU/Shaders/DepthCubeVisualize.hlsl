TextureCube<float> DepthCube : register(t0);
SamplerState LinearSampler : register(s0);
 
cbuffer DepthMapData : register(b0)
{
    matrix ViewProj;
    float4 Params; //  x=Near, y=Far, z=1/Gamma, w=unused
};

struct PSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

// 출력 구조체 
struct PSOutput
{
    float4 depth0 : SV_Target0; // +X
    float4 depth1 : SV_Target1; // -X
    float4 depth2 : SV_Target2; // +Y
    float4 depth3 : SV_Target3; // -Y
    float4 depth4 : SV_Target4; // +Z
    float4 depth5 : SV_Target5; // -Z
};

//--------------------------------------------------------------------------------------
// 헬퍼 매크로: NDC → viewZ 변환
//--------------------------------------------------------------------------------------
float LinearizeDepth(float ndc, float nearZ, float farZ, float invGamma)
{
    // Chebyshev 방식 그대로
    float A = farZ / (farZ - nearZ);
    float B = -farZ * nearZ / (farZ - nearZ);
    float viewZ = B / (ndc - A);
    float lin01 = saturate((viewZ - nearZ) / (farZ - nearZ));
    return pow(lin01, invGamma);
}

//--------------------------------------------------------------------------------------
// PS: 각 페이스 방향을 재구성해서 6번 샘플링한 뒤 6개의 RTV에 기록
//--------------------------------------------------------------------------------------
PSOutput PS_DepthVisualize(PSInput i)
{
    PSOutput o;

    // UV를 [-1, +1] 범위로 매핑
    float2 uv = i.uv * 2.0f - 1.0f;

    float nearZ = Params.x;
    float farZ = Params.y;
    float invG = Params.z;

    // +X 페이스
    {
        float3 dir = normalize(float3(1.0f, uv.y, -uv.x));
        float ndc = DepthCube.SampleLevel(LinearSampler, dir, 0).r;
        float d = LinearizeDepth(ndc, nearZ, farZ, invG);
        o.depth0 = float4(d, d, d, 1.0f);
    }

    // -X 페이스
    {
        float3 dir = normalize(float3(-1.0f, uv.y, uv.x));
        float ndc = DepthCube.SampleLevel(LinearSampler, dir, 0).r;
        float d = LinearizeDepth(ndc, nearZ, farZ, invG);
        o.depth1 = float4(d, d, d, 1.0f);
    }

    // +Y 페이스
    {
        float3 dir = normalize(float3(uv.x, 1.0f, -uv.y));
        float ndc = DepthCube.SampleLevel(LinearSampler, dir, 0).r;
        float d = LinearizeDepth(ndc, nearZ, farZ, invG);
        o.depth2 = float4(d, d, d, 1.0f);
    }

    // -Y 페이스
    {
        float3 dir = normalize(float3(uv.x, -1.0f, uv.y));
        float ndc = DepthCube.SampleLevel(LinearSampler, dir, 0).r;
        float d = LinearizeDepth(ndc, nearZ, farZ, invG);
        o.depth3 = float4(d, d, d, 1.0f);
    }

    // +Z 페이스
    {
        float3 dir = normalize(float3(uv.x, uv.y, 1.0f));
        float ndc = DepthCube.SampleLevel(LinearSampler, dir, 0).r;
        float d = LinearizeDepth(ndc, nearZ, farZ, invG);
        o.depth4 = float4(d, d, d, 1.0f);
    }

    // -Z 페이스
    {
        float3 dir = normalize(float3(-uv.x, uv.y, -1.0f));
        float ndc = DepthCube.SampleLevel(LinearSampler, dir, 0).r;
        float d = LinearizeDepth(ndc, nearZ, farZ, invG);
        o.depth5 = float4(d, d, d, 1.0f);
    }

    return o;
}
