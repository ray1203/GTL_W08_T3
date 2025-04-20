#include "SpotLightShadowMap.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Classes/Components/Light/SpotLightComponent.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/EditorEngine.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"

void FSpotLightShadowMap::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphic;
    ShaderManager = InShaderManager;

    D3D11_TEXTURE2D_DESC texDesc{};
    texDesc.Width = ShadowMapSize;
    texDesc.Height = ShadowMapSize;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32_TYPELESS;                     // Typeless
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    HRESULT hr = Graphics->Device->CreateTexture2D(&texDesc, nullptr, &DepthStencilBuffer);
    assert(SUCCEEDED(hr));

    // 2) DSV 생성 (뎁스 기록용)
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = Graphics->Device->CreateDepthStencilView(DepthStencilBuffer, &dsvDesc, &ShadowDSV);
    assert(SUCCEEDED(hr));

    // 3) SRV 생성 (쉐이더에서 깊이 읽기)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = Graphics->Device->CreateShaderResourceView(DepthStencilBuffer, &srvDesc, &ShadowSRV);
    assert(SUCCEEDED(hr));

    // 4) 비교 샘플러 생성 (쉐도우 비교 샘플링용)
    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.BorderColor[0] = 1.0f;
    sampDesc.BorderColor[1] = 1.0f;
    sampDesc.BorderColor[2] = 1.0f;
    sampDesc.BorderColor[3] = 1.0f;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

    hr = Graphics->Device->CreateSamplerState(&sampDesc, &ShadowSampler);
    assert(SUCCEEDED(hr));

    // 깊이 전용 버텍스 셰이더 가져오기
    DepthVS = ShaderManager->GetVertexShaderByKey(L"DepthOnlyVS");
    DepthIL = ShaderManager->GetInputLayoutByKey(L"DepthOnlyVS");
}

void FSpotLightShadowMap::PrepareRender()
{
    for (const auto iter : TObjectRange<ULightComponentBase>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (USpotLightComponent* PointLight = Cast<USpotLightComponent>(iter))
            {
                SpotLights.Add(PointLight);
            }
        }
    }

    // Begin Test
    if (SpotLights.Num() > 0)
    {
        FSpotLightInfo SpotLightInfo;
        SpotLightInfo.Position = SpotLights[0]->GetWorldLocation();
        SpotLightInfo.Direction = SpotLights[0]->GetDirection();

        UpdateSpotLightViewProjMatrices(SpotLightInfo);
    }
    // End Test
}

void FSpotLightShadowMap::UpdateSpotLightViewProjMatrices(const FSpotLightInfo& Info)
{
    float fovY = Info.OuterRad; // 외부 원뿔 각도
    //float fovY = Info.OuterRad * 2.0f; // 외부 원뿔 각도
    float aspectRatio = 1.0f; // 정방형 섀도우 맵
    float nearZ = 0.1f;
    float farZ = Info.Radius;

    FMatrix Projection = JungleMath::CreateProjectionMatrix(fovY, aspectRatio, nearZ, farZ);

    FVector dir = Info.Direction;
    FVector up = FVector(0, 0, 1);
    FVector target = Info.Position + dir;

    FMatrix View = JungleMath::CreateViewMatrix(Info.Position, target, up);

    SpotLightViewProjMatrix = View * Projection;
}

void FSpotLightShadowMap::RenderShadowMap()
{
    if (SpotLights.Num() <= 0) return;

    // 모든 스태틱 메시 컴포넌트 수집
    TArray<UStaticMeshComponent*> MeshComps;
    for (auto* Comp : TObjectRange<UStaticMeshComponent>())
    {
        if (Comp->GetWorld() == GEngine->ActiveWorld)
        {
            MeshComps.Add(Comp);
        }
    }

    // 뎁스 타겟 설정
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, ShadowDSV);
    Graphics->DeviceContext->ClearDepthStencilView(ShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

    D3D11_VIEWPORT vp = { 0, 0, (FLOAT)ShadowMapSize, (FLOAT)ShadowMapSize, 0, 1 };
    Graphics->DeviceContext->RSSetViewports(1, &vp);

    // 파이프라인 세팅
    Graphics->DeviceContext->VSSetShader(DepthVS, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(DepthIL);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    BufferManager->BindConstantBuffer(TEXT("FShadowViewProj"), 0, EShaderStage::Vertex);
    BufferManager->UpdateConstantBuffer(TEXT("FShadowViewProj"), SpotLightViewProjMatrix);

    // 메시 렌더
    for (auto* Comp : MeshComps)
    {
        if (!Comp->GetStaticMesh()) continue;
        auto* RenderData = Comp->GetStaticMesh()->GetRenderData();
        if (!RenderData) continue;

        // 월드 행렬 상수
        FMatrix W = Comp->GetWorldMatrix();
        BufferManager->BindConstantBuffer(TEXT("FShadowObjWorld"), 1, EShaderStage::Vertex);
        BufferManager->UpdateConstantBuffer(TEXT("FShadowObjWorld"), W);

        UINT stride = sizeof(FStaticMeshVertex);
        UINT offset = 0;
        auto* vb = RenderData->VertexBuffer;
        auto* ib = RenderData->IndexBuffer;

        Graphics->DeviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        if (ib)
        {
            Graphics->DeviceContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
            Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
        }
        else
        {
            Graphics->DeviceContext->Draw(RenderData->Vertices.Num(), 0);
        }
    }
}

void FSpotLightShadowMap::UpdateConstantBuffer()
{
    FSpotLightShadowData SpotLightShadowData;
    SpotLightShadowData.SpotLightViewProj = SpotLightViewProjMatrix;
    SpotLightShadowData.ShadowBias = 0.000f;
    BufferManager->UpdateConstantBuffer(TEXT("FPointLightShadowData"), SpotLightShadowData);
    BufferManager->BindConstantBuffer(TEXT("FPointLightShadowData"), 6, EShaderStage::Pixel);
}

void FSpotLightShadowMap::ClearRenderArr()
{
    SpotLights.Empty();
}

void FSpotLightShadowMap::SetShadowResource(int tStart)
{
    Graphics->DeviceContext->PSSetShaderResources(tStart, 1, &ShadowSRV);
}

void FSpotLightShadowMap::SetShadowSampler(int sStart)
{
    Graphics->DeviceContext->PSSetSamplers(sStart, 1, &ShadowSampler);
}

ID3D11ShaderResourceView* FSpotLightShadowMap::GetShadowSRV()
{
    return ShadowSRV;
}

