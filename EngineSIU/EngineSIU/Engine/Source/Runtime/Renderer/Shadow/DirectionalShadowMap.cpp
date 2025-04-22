#pragma once

#include "DirectionalShadowMap.h"
#include "Math/JungleMath.h"
#include "UnrealEd/PrimitiveDrawBatch.h"

#include "RendererHelpers.h"
#include "Launch/EngineLoop.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/UnrealClient.h"

#include "Engine/EditorEngine.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"

#include "Components/StaticMeshComponent.h"


FDirectionalShadowMap::~FDirectionalShadowMap()
{
    if (ShadowRasterizer)
        ShadowRasterizer->Release();
    if (ShadowSampler)
        ShadowSampler->Release();
    if (dsState)
        dsState->Release();
}

void FDirectionalShadowMap::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphic;
    ShaderManager = InShaderManager;

    CreateComparisonSampler();
    CreateShadowRasterizer();
    CreateDepthStencilState();
    LoadShadowShaders();
    CreateDepthTexture();
}

void FDirectionalShadowMap::CreateComparisonSampler()
{

    D3D11_SAMPLER_DESC comparisonSamplerDesc;
    ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
    comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.BorderColor[0] = 1.0f;
    comparisonSamplerDesc.BorderColor[1] = 1.0f;
    comparisonSamplerDesc.BorderColor[2] = 1.0f;
    comparisonSamplerDesc.BorderColor[3] = 1.0f;
    comparisonSamplerDesc.MinLOD = 0.f;
    comparisonSamplerDesc.MaxLOD = 0.f;
    comparisonSamplerDesc.MipLODBias = 0.f;
    comparisonSamplerDesc.MaxAnisotropy = 0;
    comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    Graphics->Device->CreateSamplerState(&comparisonSamplerDesc, &ShadowSampler);
}

void FDirectionalShadowMap::CreateShadowRasterizer()
{
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(rd));
    rd.CullMode = D3D11_CULL_FRONT;
    rd.FillMode = D3D11_FILL_SOLID;
    rd.FrontCounterClockwise = FALSE;
    rd.DepthBias = 100.f;
    rd.DepthBiasClamp = 0.5f;
    rd.SlopeScaledDepthBias = 0.006f;
    rd.DepthClipEnable = TRUE;
    rd.ScissorEnable = FALSE;
    rd.MultisampleEnable = FALSE;
    rd.AntialiasedLineEnable = FALSE;
    Graphics->Device->CreateRasterizerState(&rd, &ShadowRasterizer);
}

void FDirectionalShadowMap::CreateDepthStencilState()
{
    // 2) Depth-only 스텐실 상태 생성/바인딩
 /*   D3D11_DEPTH_STENCIL_DESC dsDesc{};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    Graphics->Device->CreateDepthStencilState(&dsDesc, &dsState);*/
}

void FDirectionalShadowMap::LoadShadowShaders()
{
    LightDepthOnlyVS = ShaderManager->GetVertexShaderByKey(L"LightDepthOnlyVS");
    InputLayoutLightDepthOnly = ShaderManager->GetInputLayoutByKey(L"LightDepthOnlyVS");
}


void FDirectionalShadowMap::PrepareRenderState(FEditorViewportClient* Viewport)
{
    const EViewModeIndex ViewMode = Viewport->GetViewMode();

    Graphics->DeviceContext->VSSetShader(LightDepthOnlyVS, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayoutLightDepthOnly);
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);

    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11Buffer* LightViewProj = BufferManager->GetConstantBuffer(TEXT("FLightViewProj"));
    Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &LightViewProj);
    Graphics->DeviceContext->PSSetConstantBuffers(5, 1, &LightViewProj);

}

void FDirectionalShadowMap::CollectStaticMeshes()
{
    StaticMeshComponents.Empty();
    for (const auto iter : TObjectRange<UStaticMeshComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            StaticMeshComponents.Add(iter);
        }
    }

}

void FDirectionalShadowMap::CollectDirectionalLights()
{
    DirectionalLights.Empty();

    for (const auto iter : TObjectRange<ULightComponentBase>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (UDirectionalLightComponent* DirectionalLight = Cast<UDirectionalLightComponent>(iter))
            {
                DirectionalLights.Add(DirectionalLight);
            }
        }
    }
}

void FDirectionalShadowMap::UpdateViewProjMatrices(FEditorViewportClient& ViewCamera, const FVector& LightDir)
{
    TArray<FVector> points = ViewCamera.GetFrustumCorners();

    for (auto* Comp : StaticMeshComponents)
    {
        if (!Comp || !Comp->GetStaticMesh()) continue;
        FBoundingBox bbox = Comp->GetBoundingBox();
        FMatrix     wm = Comp->GetWorldMatrix();
        for (int i = 0; i < 8; ++i)
            points.Add(bbox.TransformWorld(wm).GetCorner(i));
    }

    FVector center = FVector::ZeroVector;
    for (auto& p : points) center += p;
    center /= points.Num();

    float radius = 0.0f;
    for (auto& p : points)
    {
        float proj = FMath::Abs(FVector::DotProduct(p - center, LightDir));
        radius = FMath::Max(radius, proj);
    }

   
    FVector Eye = FVector(
        center.X - LightDir.X * radius,
        center.Y - LightDir.Y * radius,
        center.Z - LightDir.Z * radius
    );
   
    FMatrix lightView = JungleMath::CreateLookAtMatrix(Eye, center, FVector(0,0,1));
    
    FVector minB{ FLT_MAX,  FLT_MAX,  FLT_MAX };
    FVector maxB{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
    for (auto& p : points)
    {
        FVector ls = lightView.TransformPosition(p);
        minB.X = FMath::Min(minB.X, ls.X); maxB.X = FMath::Max(maxB.X, ls.X);
        minB.Y = FMath::Min(minB.Y, ls.Y); maxB.Y = FMath::Max(maxB.Y, ls.Y);
        minB.Z = FMath::Min(minB.Z, ls.Z); maxB.Z = FMath::Max(maxB.Z, ls.Z);
    }

    const float nearZ = 0.0f;
    const float farZ = maxB.Z;
    FMatrix lightProj =JungleMath::CreateOrthoOffCenterProjectionMatrix(minB.X, maxB.X, minB.Y, maxB.Y, nearZ, farZ);
   
  
    FLightViewProj vp{ lightView, lightProj };
    BufferManager->UpdateConstantBuffer(TEXT("FLightViewProj"), vp);
}

void FDirectionalShadowMap::UpdateObjectConstant(const FMatrix& WorldMatrix, const FVector4& UUIDColor, bool bIsSelected) const
{
    FObjectConstantBuffer ObjectData = {};
    ObjectData.WorldMatrix = WorldMatrix;
    ObjectData.InverseTransposedWorld = FMatrix::Transpose(FMatrix::Inverse(WorldMatrix));
    ObjectData.UUIDColor = UUIDColor;
    ObjectData.bIsSelected = bIsSelected;

    BufferManager->UpdateConstantBuffer(TEXT("FObjectConstantBuffer"), ObjectData);
}

void FDirectionalShadowMap::RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData, TArray<FStaticMaterial*> Materials, TArray<UMaterial*> OverrideMaterials, int SelectedSubMeshIndex) const
{
    UINT Stride = sizeof(FStaticMeshVertex);
    UINT Offset = 0;

    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &Stride, &Offset);

    if (RenderData->IndexBuffer)
    {
        Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }

    if (RenderData->MaterialSubsets.Num() == 0)
    {
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
        return;
    }

    for (int SubMeshIndex = 0; SubMeshIndex < RenderData->MaterialSubsets.Num(); SubMeshIndex++)
    {
        uint32 MaterialIndex = RenderData->MaterialSubsets[SubMeshIndex].MaterialIndex;

        FSubMeshConstants SubMeshData = (SubMeshIndex == SelectedSubMeshIndex) ? FSubMeshConstants(true) : FSubMeshConstants(false);

        BufferManager->UpdateConstantBuffer(TEXT("FSubMeshConstants"), SubMeshData);

        if (OverrideMaterials[MaterialIndex] != nullptr)
        {
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, OverrideMaterials[MaterialIndex]->GetMaterialInfo());
        }
        else
        {
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, Materials[MaterialIndex]->Material->GetMaterialInfo());
        }

        uint32 StartIndex = RenderData->MaterialSubsets[SubMeshIndex].IndexStart;
        uint32 IndexCount = RenderData->MaterialSubsets[SubMeshIndex].IndexCount;
        Graphics->DeviceContext->DrawIndexed(IndexCount, StartIndex, 0);
    }
}

void FDirectionalShadowMap::Render(FEditorViewportClient* Viewport)
{

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(EResourceType::ERT_Scene);

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);


  
    Graphics->DeviceContext->OMSetDepthStencilState(dsState, 0);

    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, ShadowDSV);
    Graphics->DeviceContext->ClearDepthStencilView(ShadowDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


    ChangeViewportSize();
    Graphics->DeviceContext->RSSetState(ShadowRasterizer);
    // 4) Shadow-pass 전용 셰이더 및 상수/버퍼 설정
    PrepareRenderState(Viewport);
    PrepareRender(Viewport);

    // 5) 모든 StaticMeshComponent에 대해 Depth-only DrawIndexed
    for (UStaticMeshComponent* Comp : StaticMeshComponents)
    {
        if (!Comp || !Comp->GetStaticMesh()) continue;
        auto* RD = Comp->GetStaticMesh()->GetRenderData();
        if (!RD) continue;

        // 세팅: 월드 매트릭스만 업데이트
        UpdateObjectConstant(Comp->GetWorldMatrix(), Comp->EncodeUUID() / 255.0f, false);

        // 실제로 Depth-only 드로우
        RenderPrimitive(RD,
            Comp->GetStaticMesh()->GetMaterials(),
            Comp->GetOverrideMaterials(),
            Comp->GetselectedSubMeshIndex());
    }

    // 6) DSV 언바인드 및 상태 해제
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = ViewportResource->GetD3DViewport().Width;
    vp.Height = ViewportResource->GetD3DViewport().Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    Graphics->DeviceContext->RSSetViewports(1, &vp);
    ImGui::Image((ImTextureID)ShadowSRV, ImVec2((float)512, (float)512));

    Graphics->DeviceContext->PSSetShaderResources(5, 1, &ShadowSRV);
    Graphics->DeviceContext->PSSetSamplers(5, 1, &ShadowSampler);
    Graphics->DeviceContext->RSSetState(nullptr);

}

void FDirectionalShadowMap::ChangeViewportSize()
{
    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = ShadowResolution;
    vp.Height = ShadowResolution;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    Graphics->DeviceContext->RSSetViewports(1, &vp);
}

void FDirectionalShadowMap::CreateDepthTexture()
{
    // 1) 기존 리소스 해제
    if (DepthStencilTexture) { DepthStencilTexture->Release(); DepthStencilTexture = nullptr; }
    if (ShadowDSV) { ShadowDSV->Release();           ShadowDSV = nullptr; }
    if (ShadowSRV) { ShadowSRV->Release();           ShadowSRV = nullptr; }


    D3D11_TEXTURE2D_DESC shadowMapDesc;
    ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
    shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    shadowMapDesc.MipLevels = 0;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
    shadowMapDesc.CPUAccessFlags = 0;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.SampleDesc.Quality = 0;
    shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    shadowMapDesc.Height = (UINT)ShadowResolution;
    shadowMapDesc.Width = (UINT)ShadowResolution;
    FEngineLoop::GraphicDevice.Device->CreateTexture2D(&shadowMapDesc, nullptr, &DepthStencilTexture);


    // 2-2) Depth-Stencil View (DSV)
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(DepthStencilTexture, &depthStencilViewDesc, &ShadowDSV);

    // 2-3) Shader Resource View (SRV)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    Graphics->Device->CreateShaderResourceView(DepthStencilTexture, &srvDesc, &ShadowSRV);

}

void FDirectionalShadowMap::PrepareRender(FEditorViewportClient* Viewport)
{
    CollectStaticMeshes();
    CollectDirectionalLights();

    int DirectionalLightsCount = 0;
    for (auto Light : DirectionalLights)
    {
        if (DirectionalLightsCount < MAX_DIRECTIONAL_LIGHT)
        {
            Light->GetDirectionalLightInfo().Direction;
            UpdateViewProjMatrices(*Viewport, Light->GetDirection());
            DirectionalLightsCount++;
        }
    }

}
