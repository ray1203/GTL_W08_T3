#pragma once

#include "CascadeShadowMap.h"
#include "Math/JungleMath.h"
#include "UnrealEd/PrimitiveDrawBatch.h"
#include "Launch/EngineLoop.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/UnrealClient.h"


void FCascadeShadowMap::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphic;
    ShaderManager = InShaderManager;

    //TODO UnrealClient의 뎁스를 가지고 와야함
    
    //// 1) Typeless 텍스처: DSV와 SRV 모두 사용
    //D3D11_TEXTURE2D_DESC texDesc{};
    //texDesc.Width = 1920;
    //texDesc.Height = 1080;
    //texDesc.MipLevels = 1;
    //texDesc.ArraySize = 1;
    //texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    //texDesc.SampleDesc.Count = 1;
    //texDesc.Usage = D3D11_USAGE_DEFAULT;
    //texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    //Graphics->Device->CreateTexture2D(&texDesc, nullptr, &DepthStencilBuffer);
    //Graphics->BackBufferTexture;
    // 2) DSV: 깊이 기록용
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    Graphics->Device->CreateDepthStencilView(DepthStencilBuffer, &dsvDesc, &ShadowDSV);

    // 3) SRV: 읽기용
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    Graphics->Device->CreateShaderResourceView(DepthStencilBuffer, &srvDesc, &ShadowSRV);

    // 4) 샘플러: 일반 샘플링
    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    Graphics->Device->CreateSamplerState(&sampDesc, &ShadowSampler);

    // 5) 셰이더 로드
    ShaderManager->AddVertexShader(L"DepthBufferVertexShader", L"Shaders/DepthBufferVertexShader.hlsl", "mainVS");
    ShaderManager->AddPixelShader(L"DepthBufferPixelShader", L"Shaders/DepthBufferPixelShader.hlsl", "mainPS");
}

void FCascadeShadowMap::UpdateCascadeViewProjMatrices(
    FEditorViewportClient& ViewCamera,
    const FVector& LightDir)
{
    // 1) 카메라 프러스텀 코너 획득
    TArray<FVector> corners = ViewCamera.GetFrustumCorners();
    FVector center = ComputeFrustumCenter(corners);

    // 2) 라이트 뷰 매트릭스
    float distance = ViewCamera.FarClip * 1.5f;
    FVector lightPos = center - LightDir * distance;
    FMatrix lightView = JungleMath::CreateViewMatrix(lightPos, center, FVector(0, 0, 1));

    // 3) 라이트 공간에서 익스트림 계산
    float minX = FLT_MAX, maxX = -FLT_MAX;
    float minY = FLT_MAX, maxY = -FLT_MAX;
    float minZ = FLT_MAX, maxZ = -FLT_MAX;
    for (auto& c : corners)
    {
        FVector lp = lightView.TransformPosition(c);
        minX = FMath::Min(minX, lp.X); maxX = FMath::Max(maxX, lp.X);
        minY = FMath::Min(minY, lp.Y); maxY = FMath::Max(maxY, lp.Y);
        minZ = FMath::Min(minZ, lp.Z); maxZ = FMath::Max(maxZ, lp.Z);
    }

    // 4) 오프센터 정투영
    FMatrix lightProj = JungleMath::CreateOrthoOffCenterProjectionMatrix(
        minX, maxX, minY, maxY, FMath::Max(0.f, minZ), maxZ);

    CascadeViewProjMatrix = lightView * lightProj;

    // (다중 캐스케이드 사용 시, Cascades 배열에 각 뷰프로젝션 저장)
}

void FCascadeShadowMap::Render(FEditorViewportClient* Viewport)
{
    // === 1) Depth Pass: 오직 DSV에 그림자 깊이만 기록 ===

    // 1-1) 그림자 깊이용 DSV 바인딩, 컬러 타겟은 언바인딩
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, ShadowDSV);

    // 1-2) 깊이 버퍼 초기화 (클리어)
    Graphics->DeviceContext->ClearDepthStencilView(
        ShadowDSV,
        D3D11_CLEAR_DEPTH,
        1.0f,
        0
    );

    // 1-3) 깊이 쓰기용 스텐실 상태 생성 및 바인딩
    D3D11_DEPTH_STENCIL_DESC dsWriteDesc{};
    dsWriteDesc.DepthEnable = TRUE;
    dsWriteDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsWriteDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    ID3D11DepthStencilState* dsWriteState = nullptr;
    Graphics->Device->CreateDepthStencilState(&dsWriteDesc, &dsWriteState);
    Graphics->DeviceContext->OMSetDepthStencilState(dsWriteState, 0);

    // 1-4) 그림자용 셰이더 바인딩
    Graphics->DeviceContext->VSSetShader(
        ShaderManager->GetVertexShaderByKey(L"DepthBufferVertexShader"),
        nullptr, 0
    );
    Graphics->DeviceContext->PSSetShader(
        ShaderManager->GetPixelShaderByKey(L"DepthBufferPixelShader"),
        nullptr, 0
    );

    // 1-5) 드로우 설정 및 호출 (풀스크린 쿼드 또는 씬 지오메트리)
    Graphics->DeviceContext->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    );
    Graphics->DeviceContext->Draw(6, 0);

    // 1-6) 상태 해제
    Graphics->DeviceContext->OMSetDepthStencilState(nullptr, 0);
    if (dsWriteState) { dsWriteState->Release(); }


    // === 2) Debug Pass: 백버퍼에 깊이맵 시각화 ===

    // 2-1) DSV 언바인딩
    ID3D11DepthStencilView* nullDSV = nullptr;
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullDSV);

    // 2-2) 백버퍼(RTV) 바인딩
    const EResourceType ResourceType = EResourceType::ERT_Scene;
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    FRenderTargetRHI* RT = ViewportResource->GetRenderTarget(ResourceType);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RT->RTV, nullptr);

       Graphics->DeviceContext->VSSetShader(
        ShaderManager->GetVertexShaderByKey(L"DepthBufferVertexShader"),
        nullptr, 0
    );
    //     PS: DebugDepthPixelShader (깊이값→흑백 컬러)
    Graphics->DeviceContext->PSSetShader(
        ShaderManager->GetPixelShaderByKey(L"DebugDepthPixelShader"),
        nullptr, 0
    );

    // 2-4) SRV와 샘플러 바인딩
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &ShadowSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &ShadowSampler);
    Graphics->DeviceContext->IASetInputLayout(nullptr);

    // 2-5) 풀스크린 쿼드 드로우 (삼각형 2개나 3개)
    Graphics->DeviceContext->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    );
    Graphics->DeviceContext->Draw(6, 0);

    // 2-6) SRV 바인딩 해제
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
}



const TArray<FCascade>& FCascadeShadowMap::GetCascades() const
{
    return Cascades;
}
