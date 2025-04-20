#pragma once

#include "CascadeShadowMap.h"
#include "Math/JungleMath.h"
#include "UnrealEd/PrimitiveDrawBatch.h"
#include "Launch/EngineLoop.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/UnrealClient.h"

#include "Engine/EditorEngine.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"


void FDirectionalShadowMap::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphic;
    ShaderManager = InShaderManager;


    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    Graphics->Device->CreateSamplerState(&sampDesc, &ShadowSampler);


}
void FDirectionalShadowMap::UpdateCascadeViewProjMatrices(FEditorViewportClient& ViewCamera, const FVector& LightDir)
{
    // 1) 카메라 프러스텀 코너 획득
    auto corners = ViewCamera.GetFrustumCorners();
    FVector center = ComputeFrustumCenter(corners);

    // 카메라 FarClip 거리 사용
    float dist = ViewCamera.FarClip;

    // lightView, lightProj 생성
    FMatrix lightView = JungleMath::CreateLightViewMatrix(center, LightDir, dist);
  
    // light-space extents 계산
    float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX;
    float minZ = FLT_MAX, maxZ = -FLT_MAX;
    for (auto& c : corners)
    {
        FVector lp = lightView.TransformPosition(c);
        minX = FMath::Min(minX, lp.X);  maxX = FMath::Max(maxX, lp.X);
        minY = FMath::Min(minY, lp.Y);  maxY = FMath::Max(maxY, lp.Y);
        minZ = FMath::Min(minZ, lp.Z);  maxZ = FMath::Max(maxZ, lp.Z);
    }

    float nearPlane = FMath::Max(minZ, 0.0001f);

    FMatrix lightProj = JungleMath::CreateOrthoOffCenterProjectionMatrix(
        minX, maxX, minY, maxY, nearPlane, maxZ);

    CascadeViewProjMatrix = lightView * lightProj;


    ID3D11Buffer* LightViewProj = BufferManager->GetConstantBuffer(TEXT("FLightViewProj"));
    Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &LightViewProj);
    Graphics->DeviceContext->PSSetConstantBuffers(5, 1, &LightViewProj);
    FLightViewProj LightViewProjData = { CascadeViewProjMatrix };
    BufferManager->UpdateConstantBuffer(TEXT("FLightViewProj"), LightViewProjData);

}
void FDirectionalShadowMap::Render(FEditorViewportClient* Viewport)
{
 
    //// 2-2) 백버퍼(RTV) 바인딩
    //const EResourceType ResourceType = EResourceType::ERT_Scene;
    //FViewportResource* ViewportResource = Viewport->GetViewportResource();
    //FRenderTargetRHI* RT = ViewportResource->GetRenderTarget(ResourceType);
    //Graphics->DeviceContext->OMSetRenderTargets(1, &RT->RTV, nullptr);

    //Graphics->DeviceContext->VSSetShader(
    //    ShaderManager->GetVertexShaderByKey(L"DepthBufferVertexShader"),
    //    nullptr, 0
    //);
    ////     PS: DebugDepthPixelShader (깊이값→흑백 컬러)
    //Graphics->DeviceContext->PSSetShader(
    //    ShaderManager->GetPixelShaderByKey(L"DebugDepthPixelShader"),
    //    nullptr, 0
    //);

    //// 2-4) SRV와 샘플러 바인딩
    //Graphics->DeviceContext->PSSetShaderResources(0, 1, &ShadowSRV);
    //Graphics->DeviceContext->PSSetSamplers(0, 1, &ShadowSampler);
    //Graphics->DeviceContext->IASetInputLayout(nullptr);

    //// 2-5) 풀스크린 쿼드 드로우 (삼각형 2개나 3개)
    //Graphics->DeviceContext->IASetPrimitiveTopology(
    //    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    //);
    //Graphics->DeviceContext->Draw(6, 0);

    //// 2-6) SRV 바인딩 해제
    //ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    //Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
}

void FDirectionalShadowMap::PrepareRender()
{

}

void FDirectionalShadowMap::ResizeTexture(FEditorViewportClient* ViewCamera)
{
    uint32 newW = 2048;
    uint32 newH = 2048;
    if (Width != newW || Height != newH)
    {
        // 1) 기존 리소스 해제
        if (DepthStencilTexture) { DepthStencilTexture->Release(); DepthStencilTexture = nullptr; }
        if (ShadowDSV) { ShadowDSV->Release();           ShadowDSV = nullptr; }
        if (ShadowSRV) { ShadowSRV->Release();           ShadowSRV = nullptr; }

        // 2) 새로운 텍스처/뷰 생성
        // 2-1) Depth-Stencil 텍스처
        D3D11_TEXTURE2D_DESC texDesc{};
        texDesc.Width = newW;
        texDesc.Height = newH;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        FEngineLoop::GraphicDevice.Device->CreateTexture2D(&texDesc, nullptr, &DepthStencilTexture);

        // 2-2) Depth-Stencil View (DSV)
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(DepthStencilTexture, &dsvDesc, &ShadowDSV);

        // 2-3) Shader Resource View (SRV)
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        Graphics->Device->CreateShaderResourceView(DepthStencilTexture, &srvDesc, &ShadowSRV);
         
        Width = newW;
        Height = newH;
    }
}

void FDirectionalShadowMap::PrepareRender(FEditorViewportClient* Viewport)
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
    int DirectionalLightsCount = 0;
    for (auto Light : DirectionalLights)
    {
        if (DirectionalLightsCount < MAX_DIRECTIONAL_LIGHT)
        {
            Light->GetDirectionalLightInfo().Direction;
            UpdateCascadeViewProjMatrices(*Viewport, Light->GetDirection());
            DirectionalLightsCount++;
        }
    }

}



const TArray<FCascade>& FDirectionalShadowMap::GetCascades() const
{
    return Cascades;
}
