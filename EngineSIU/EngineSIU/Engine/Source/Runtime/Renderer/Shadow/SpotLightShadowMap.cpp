#include "SpotLightShadowMap.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Classes/Components/Light/SpotLightComponent.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/EditorEngine.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"

void FSpotLightShadowMap::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager, EShadowFilter InShadowFilter)
{
    BufferManager = InBufferManager;
    Graphics = InGraphic;
    ShaderManager = InShaderManager;
    ShadowFilter = InShadowFilter;

    HRESULT hr;

    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
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

    D3D11_SAMPLER_DESC sampDescVSM = {};
    sampDescVSM.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    sampDescVSM.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDescVSM.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDescVSM.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDescVSM.BorderColor[0] = 1.0f;
    sampDescVSM.BorderColor[1] = 1.0f;
    sampDescVSM.BorderColor[2] = 1.0f;
    sampDescVSM.BorderColor[3] = 1.0f;
    sampDescVSM.MinLOD = 0;
    sampDescVSM.MaxLOD = D3D11_FLOAT32_MAX;
    sampDescVSM.ComparisonFunc = D3D11_COMPARISON_NEVER;

    hr = Graphics->Device->CreateSamplerState(&sampDescVSM, &ShadowSamplerVSM);
    assert(SUCCEEDED(hr));

    // 깊이 전용 버텍스 셰이더 가져오기
    DepthVS = ShaderManager->GetVertexShaderByKey(L"DepthOnlyVS");
    DepthIL = ShaderManager->GetInputLayoutByKey(L"DepthOnlyVS");

    InitializeDebugVisualizationResources();
}

void FSpotLightShadowMap::InitializeDebugVisualizationResources()
{
    HRESULT hr;

    D3D11_TEXTURE2D_DESC linDesc = {};
    linDesc.Width = ShadowMapSize;
    linDesc.Height = ShadowMapSize;
    linDesc.MipLevels = 1;
    linDesc.ArraySize = 1;
    linDesc.Format = DXGI_FORMAT_R32_TYPELESS;  // 또는 직접 R32_FLOAT 로 해도 무방
    linDesc.SampleDesc = { 1,0 };
    linDesc.Usage = D3D11_USAGE_DEFAULT;
    linDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    linDesc.CPUAccessFlags = 0;
    linDesc.MiscFlags = 0;

    //

    hr = Graphics->Device->CreateTexture2D(&linDesc, nullptr, &DebugDepthLinearBuffer);
    assert(SUCCEEDED(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = Graphics->Device->CreateShaderResourceView(DebugDepthLinearBuffer, &srvDesc, &ShadowDebugSRV);
    assert(SUCCEEDED(hr));

    //

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    hr = Graphics->Device->CreateRenderTargetView(DebugDepthLinearBuffer, &rtvDesc, &ShadowDebugRTV);
    assert(SUCCEEDED(hr));

    //

    D3D11_SAMPLER_DESC sampLinearDesc = {};
    sampLinearDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampLinearDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampLinearDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampLinearDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampLinearDesc.MipLODBias = 0.0f;
    sampLinearDesc.MaxAnisotropy = 1;
    sampLinearDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;      // 일반 샘플링용
    sampLinearDesc.MinLOD = 0;
    sampLinearDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = Graphics->Device->CreateSamplerState(&sampLinearDesc, &DebugSampler);
    assert(SUCCEEDED(hr));

    if (ShadowFilter == EShadowFilter::ESF_VSM)
    {
        // VSM 생성용 픽셀 셰이더 로드
        VSMGenerationPS = ShaderManager->GetPixelShaderByKey(L"VSMGenerationPS");
        assert(VSMGenerationPS != nullptr);
    }

    FullscreenVS = ShaderManager->GetVertexShaderByKey(L"FullScreenVS");
    FullscreenIL = ShaderManager->GetInputLayoutByKey(L"FullScreenVS");
    DepthVisualizePS = ShaderManager->GetPixelShaderByKey(L"DepthVisualizePS");
}

void FSpotLightShadowMap::PrepareRender()
{
    for (const auto iter : TObjectRange<ULightComponentBase>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(iter))
            {
                SpotLights.Add(SpotLight);
            }
        }
    }

    int spotLightNum = SpotLights.Num();
    if (prevSpotNum < spotLightNum) 
    {
        AddSpotLightResource(spotLightNum - prevSpotNum);
    }
    else if (prevSpotNum > spotLightNum) 
    {
        DeleteSpotLightResource(prevSpotNum - spotLightNum);
    }

    prevSpotNum = spotLightNum;

    //if (SpotLights.Num() > 0)
    //{
    //    FSpotLightInfo SpotLightInfo;
    //    SpotLightInfo.Position = SpotLights[0]->GetWorldLocation();
    //    SpotLightInfo.Direction = SpotLights[0]->GetDirection();

    //    UpdateSpotLightViewProjMatrices(SpotLightInfo);
    //}

    // Begin Test
    for (int i = 0; i < SpotLights.Num(); i++)
    {
        FSpotLightInfo SpotLightInfo;
        SpotLightInfo.Position = SpotLights[i]->GetWorldLocation();
        SpotLightInfo.Direction = SpotLights[i]->GetDirection();
        UpdateSpotLightViewProjMatrices(i, SpotLightInfo);
    }
    // End Test
}

void FSpotLightShadowMap::UpdateSpotLightViewProjMatrices(int index, const FSpotLightInfo& Info)
{
    //float fovY = Info.OuterRad; // 외부 원뿔 각도
    float fovY = Info.OuterRad * 2.0f; // 외부 원뿔 각도
    float aspectRatio = 1.0f; // 정방형 섀도우 맵
    float nearZ = 0.1f;
    float farZ = Info.Radius;
    // testtest
    FMatrix Projection = JungleMath::CreateProjectionMatrix(FMath::DegreesToRadians(90), aspectRatio, nearZ, farZ);

    FVector dir = Info.Direction;

    // 더 안정적인 up 벡터 계산 방법
    FVector up;
    // dir과 월드 up(0,0,1) 벡터가 거의 평행한 경우를 처리
    if (FMath::Abs(dir.Z) > 0.99f)
    {
        // dir이 거의 수직이면 월드 X축(1,0,0)을 사용
        up = FVector(1.0f, 0.0f, 0.0f);
    }
    else
    {
        // 일반적인 경우 월드 up 벡터 사용
        up = FVector(0.0f, 0.0f, 1.0f);
    }

    // dir과 직교하는 실제 up 벡터 계산
    FVector right = FVector::CrossProduct(up, dir).GetSafeNormal();
    up = FVector::CrossProduct(dir, right).GetSafeNormal();

    FVector target = Info.Position + dir;

    FMatrix View = JungleMath::CreateViewMatrix(Info.Position, target, up);

    SpotLightShadowResources[index].SpotLightViewProjMatrix = View * Projection;
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

    for (int i = 0; i < SpotLightShadowResources.Num(); i++)
    {
        FSpotLightShadowResource& res = SpotLightShadowResources[i];

        // Set Viewport
        D3D11_VIEWPORT vp = { 0, 0, (FLOAT)ShadowMapSize, (FLOAT)ShadowMapSize, 0, 1 };
        Graphics->DeviceContext->RSSetViewports(1, &vp);

        // Set Common Pipeline States
        Graphics->DeviceContext->IASetInputLayout(DepthIL);
        Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        Graphics->DeviceContext->VSSetShader(DepthVS, nullptr, 0);

        // Update Constant Buffers
        BufferManager->BindConstantBuffer(TEXT("FShadowViewProj"), 0, EShaderStage::Vertex);
        BufferManager->UpdateConstantBuffer(TEXT("FShadowViewProj"), res.SpotLightViewProjMatrix);
        BufferManager->BindConstantBuffer(TEXT("FShadowObjWorld"), 1, EShaderStage::Vertex); // Bind once outside mesh loop

        if (ShadowFilter == EShadowFilter::ESF_VSM)
        {
            // --- VSM Rendering Path ---
            // Set Render Target (VSM Texture) and Depth Buffer (Separate)
            ID3D11RenderTargetView* rtvs[] = { res.VSMRTV };
            Graphics->DeviceContext->OMSetRenderTargets(1, rtvs, res.VSMDepthDSV);

            // Clear VSM Render Target (depth=1, depth^2=1) and Depth Buffer
            const float clearColorVSM[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            Graphics->DeviceContext->ClearRenderTargetView(res.VSMRTV, clearColorVSM);
            Graphics->DeviceContext->ClearDepthStencilView(res.VSMDepthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

            // Set VSM Pixel Shader
            Graphics->DeviceContext->PSSetShader(VSMGenerationPS, nullptr, 0);
        }
        else // Standard PCF or other non-VSM filter
        {
            Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, res.ShadowDSV);
            Graphics->DeviceContext->ClearDepthStencilView(res.ShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

            Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
        }

        // Render Meshes (Loop remains the same)
        for (auto* Comp : MeshComps)
        {
            if (!Comp->GetStaticMesh()) continue;
            auto* RenderData = Comp->GetStaticMesh()->GetRenderData();
            if (!RenderData || RenderData->Vertices.Num() == 0) continue; // Check for valid render data

            FMatrix W = Comp->GetWorldMatrix();
            BufferManager->UpdateConstantBuffer(TEXT("FShadowObjWorld"), W);

            UINT stride = sizeof(FStaticMeshVertex);
            UINT offset = 0;
            auto* vb = RenderData->VertexBuffer;
            auto* ib = RenderData->IndexBuffer;

            Graphics->DeviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
            if (ib && RenderData->Indices.Num() > 0) // Check for valid index buffer
            {
                Graphics->DeviceContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
                Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
            }
        }

        // --- Unbind Render Targets / Resources ---
        // Important if the generated shadow map is immediately used as SRV
        ID3D11RenderTargetView* nullRTVs[] = { nullptr };
        Graphics->DeviceContext->OMSetRenderTargets(1, nullRTVs, nullptr);
    }

}


void FSpotLightShadowMap::ClearRenderArr()
{
    SpotLights.Empty();
}

// 근데 지금 여기서 VSM까지 넘겨주나
// 이거 잘못됐네 ㅇㅇ
// SetShadowVSMResource로 하거나 따로 분기를 둬야할 것 같음.
void FSpotLightShadowMap::SetShadowResource(int tStart)
{
    for (int i = 0; i < SpotLightShadowResources.Num(); i++) 
    {
        Graphics->DeviceContext->PSSetShaderResources(tStart + i, 1, &SpotLightShadowResources[i].ShadowSRV);
    }
}
void FSpotLightShadowMap::SetShadowResources(int pcfTextureSlotStart, int vsmTextureSlotStart)
{
    for (int i = 0; i < SpotLightShadowResources.Num(); i++)
    {
        if (SpotLightShadowResources[i].ShadowSRV)
        {
            Graphics->DeviceContext->PSSetShaderResources(
                pcfTextureSlotStart + i, 
                1,
                &SpotLightShadowResources[i].ShadowSRV
            );
        }

        if (SpotLightShadowResources[i].VSMSRV)
        {
            Graphics->DeviceContext->PSSetShaderResources(
                vsmTextureSlotStart + i, // VSM 시작 슬롯 + 오프셋
                1,
                &SpotLightShadowResources[i].VSMSRV
            );
        }
    }
}


void FSpotLightShadowMap::SetShadowSampler(int sStart)
{
    Graphics->DeviceContext->PSSetSamplers(sStart, 1, &ShadowSampler);
}

void FSpotLightShadowMap::SetShadowFilterSampler(int sStart)
{
    Graphics->DeviceContext->PSSetSamplers(sStart, 1, &ShadowSamplerVSM);
}

ID3D11ShaderResourceView* FSpotLightShadowMap::GetShadowSRV(int index)
{
    return SpotLightShadowResources[index].ShadowSRV;
}

void FSpotLightShadowMap::RenderLinearDepth()
{
    // 일단 0번 Render
    // TODO 나중에 고치기

    if (SpotLightShadowResources[0].DepthStencilBuffer == nullptr) return;

    // ─── 0) 기존 RenderTargets, DepthStencilView, Viewports 백업 ───
    ID3D11RenderTargetView* oldRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
    ID3D11DepthStencilView* oldDSV = nullptr;
    Graphics->DeviceContext->OMGetRenderTargets(
        D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, oldRTVs, &oldDSV);

    UINT numVP = D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX;
    D3D11_VIEWPORT oldVPs[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
    Graphics->DeviceContext->RSGetViewports(&numVP, oldVPs);

    BufferManager->BindConstantBuffer(TEXT("FDepthMapData"), 0, EShaderStage::Pixel);

    // (A) RTV 세팅 & 클리어
    ID3D11RenderTargetView* rtvs[] = { ShadowDebugRTV };
    Graphics->DeviceContext->OMSetRenderTargets(1, rtvs, nullptr);
    const float clearColor[4] = { 0, 0, 0, 0 };
    Graphics->DeviceContext->ClearRenderTargetView(ShadowDebugRTV, clearColor);

    // (B) 뷰포트 설정
    D3D11_VIEWPORT vp = { 0.f, 0.f, (float)ShadowMapSize, (float)ShadowMapSize, 0.f, 1.f };
    Graphics->DeviceContext->RSSetViewports(1, &vp);

    // (C) 풀스크린 파이프라인 바인딩
    Graphics->DeviceContext->IASetInputLayout(FullscreenIL);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->VSSetShader(FullscreenVS, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(DepthVisualizePS, nullptr, 0);

    // (D) 원본 Depth SRV 와 리니어 샘플러 바인딩
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &SpotLightShadowResources[0].ShadowSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &DebugSampler);

    // (E) 카메라(라이트) 매트릭스 및 Near/Far/Gamma 상수 업데이트
    FDepthMapData depthMapData;
    depthMapData.ViewProj = SpotLightShadowResources[0].SpotLightViewProjMatrix;           // light ViewProj
    depthMapData.Params.X = 0.1f;                                     // Near plane
    // TODO Light의 범위를 저장해 뒀다가 Far Plane 값에 적용 필요함
    // 일단 임시로 20 값을 넣어 뒀음
    depthMapData.Params.Y = 20.0f;                   // Far plane = Light Radius
    depthMapData.Params.Z = 1.0f / 2.2f;                             // invGamma (예: gamma=2.2)
    depthMapData.Params.W = 0;
    BufferManager->UpdateConstantBuffer(TEXT("FDepthMapData"), depthMapData);

    // (F) 풀스크린 삼각형 드로우
    Graphics->DeviceContext->Draw(3, 0);

    // ─── 3) 이전 RTV/DSV & Viewports 복구 ───────────────
    Graphics->DeviceContext->RSSetViewports(numVP, oldVPs);
    Graphics->DeviceContext->OMSetRenderTargets(
        D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
        oldRTVs, oldDSV);

    // ─── 4) Release 참조 카운트 낮추기 ─────────────────
    for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        if (oldRTVs[i]) oldRTVs[i]->Release();
    if (oldDSV) oldDSV->Release();
}

void FSpotLightShadowMap::AddSpotLightResource(int num)
{
    for (int i = 0; i < num; i++) 
    {
        FSpotLightShadowResource spotLightShadowResource;

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

        HRESULT hr = Graphics->Device->CreateTexture2D(&texDesc, nullptr, &spotLightShadowResource.DepthStencilBuffer);
        assert(SUCCEEDED(hr));

        // 2) DSV 생성 (뎁스 기록용)
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        hr = Graphics->Device->CreateDepthStencilView(spotLightShadowResource.DepthStencilBuffer, &dsvDesc, &spotLightShadowResource.ShadowDSV);
        assert(SUCCEEDED(hr));

        // 3) SRV 생성 (쉐이더에서 깊이 읽기)
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = Graphics->Device->CreateShaderResourceView(spotLightShadowResource.DepthStencilBuffer, &srvDesc, &spotLightShadowResource.ShadowSRV);
        assert(SUCCEEDED(hr));

        D3D11_TEXTURE2D_DESC texDescVSM = {};
        texDescVSM.Width = ShadowMapSize;
        texDescVSM.Height = ShadowMapSize;
        texDescVSM.MipLevels = 1;
        texDescVSM.ArraySize = 1;
        texDescVSM.Format = DXGI_FORMAT_R32G32_FLOAT; 
        texDescVSM.SampleDesc.Count = 1;
        texDescVSM.Usage = D3D11_USAGE_DEFAULT;
        texDescVSM.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // Bind as RT and SRV
        texDescVSM.CPUAccessFlags = 0;
        texDescVSM.MiscFlags = 0;

        hr = Graphics->Device->CreateTexture2D(&texDescVSM, nullptr, &spotLightShadowResource.VSMTexture);
        assert(SUCCEEDED(hr));

        // Begin Test

        // VSM Render Target View
        D3D11_RENDER_TARGET_VIEW_DESC rtvDescVSM = {};
        rtvDescVSM.Format = texDescVSM.Format;
        rtvDescVSM.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDescVSM.Texture2D.MipSlice = 0;
        hr = Graphics->Device->CreateRenderTargetView(spotLightShadowResource.VSMTexture, &rtvDescVSM, &spotLightShadowResource.VSMRTV);
        assert(SUCCEEDED(hr));

        // VSM Shader Resource View
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDescVSM = {};
        srvDescVSM.Format = texDescVSM.Format;
        srvDescVSM.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDescVSM.Texture2D.MostDetailedMip = 0;
        srvDescVSM.Texture2D.MipLevels = 1;
        hr = Graphics->Device->CreateShaderResourceView(spotLightShadowResource.VSMTexture, &srvDescVSM, &spotLightShadowResource.VSMSRV);
        assert(SUCCEEDED(hr));

        // VSM Depth Buffer (Separate)
        D3D11_TEXTURE2D_DESC depthDescVSM = {};
        depthDescVSM.Width = ShadowMapSize;
        depthDescVSM.Height = ShadowMapSize;
        depthDescVSM.MipLevels = 1;
        depthDescVSM.ArraySize = 1;
        depthDescVSM.Format = DXGI_FORMAT_D32_FLOAT; // Standard depth format
        depthDescVSM.SampleDesc.Count = 1;
        depthDescVSM.Usage = D3D11_USAGE_DEFAULT;
        depthDescVSM.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthDescVSM.CPUAccessFlags = 0;
        depthDescVSM.MiscFlags = 0;

        hr = Graphics->Device->CreateTexture2D(&depthDescVSM, nullptr, &spotLightShadowResource.VSMDepthBuffer);
        assert(SUCCEEDED(hr));

        // VSM Depth Stencil View
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDescVSM = {};
        dsvDescVSM.Format = depthDescVSM.Format;
        dsvDescVSM.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDescVSM.Texture2D.MipSlice = 0;
        hr = Graphics->Device->CreateDepthStencilView(spotLightShadowResource.VSMDepthBuffer, &dsvDescVSM, &spotLightShadowResource.VSMDepthDSV);
        assert(SUCCEEDED(hr));
        // End Test

        SpotLightShadowResources.Add(spotLightShadowResource);
    }
}

void FSpotLightShadowMap::DeleteSpotLightResource(int num)
{
    for (int i = prevSpotNum - 1; i >= prevSpotNum - num; i--) 
    {
        if (SpotLightShadowResources[i].DepthStencilBuffer) SpotLightShadowResources[i].DepthStencilBuffer->Release();
        if (SpotLightShadowResources[i].ShadowSRV) SpotLightShadowResources[i].ShadowSRV->Release();
        if (SpotLightShadowResources[i].ShadowDSV) SpotLightShadowResources[i].ShadowDSV->Release();

        if (SpotLightShadowResources[i].VSMTexture) SpotLightShadowResources[i].VSMTexture->Release();
        if (SpotLightShadowResources[i].VSMRTV) SpotLightShadowResources[i].VSMRTV->Release();
        if (SpotLightShadowResources[i].VSMSRV) SpotLightShadowResources[i].VSMSRV->Release();
        if (SpotLightShadowResources[i].VSMDepthBuffer) SpotLightShadowResources[i].VSMDepthBuffer->Release();
        if (SpotLightShadowResources[i].VSMDepthDSV) SpotLightShadowResources[i].VSMDepthDSV->Release();


        SpotLightShadowResources.RemoveAt(i);
    }
}

ID3D11ShaderResourceView* FSpotLightShadowMap::GetShadowDebugSRV()
{
    return ShadowDebugSRV;
}

FMatrix FSpotLightShadowMap::GetViewProjMatrix(int index)
{
    return SpotLightShadowResources[index].SpotLightViewProjMatrix;
}
