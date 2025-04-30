#include "CameraPostProcess.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "UObject/UObjectIterator.h"
#include "RendererHelpers.h"
#include "Editor/UnrealEd/EditorViewportClient.h"
#include "Engine/Camera/PlayerCameraManager.h"
#include "Engine/Engine.h"
#include "World/World.h"

FCameraPostProcess::FCameraPostProcess()
{

}

FCameraPostProcess::~FCameraPostProcess()
{
    ReleaseShader();
}

void FCameraPostProcess::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    Graphics = InGraphics;
    BufferManager = InBufferManager;
    ShaderManager = InShaderManager;
    CreateShader();
    CreateConstantBuffer();
}

void FCameraPostProcess::PrepareRender()
{
    if (UWorld* World = GEngine->ActiveWorld)
    {
        if (const APlayerCameraManager* PlayerCameraManager = World->GetPlayerCameraManager())
        {
            FadeParams.FadeAmount = PlayerCameraManager->FadeAmount;
            FadeParams.FadeColor = PlayerCameraManager->FadeColor;

            LetterBoxParams.BoxColor = FLinearColor(0, 0, 0, 1);
            LetterBoxParams.LetterBoxRatio = PlayerCameraManager->LetterBoxRatio;
        }
    }
    
}

void FCameraPostProcess::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    // RenderTarget 공유 

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    const EResourceType ResourceType = EResourceType::ERT_PP_Camera;
    FRenderTargetRHI* RenderTargetRHI = Viewport->GetViewportResource()->GetRenderTarget(ResourceType);

    ViewportResource->ClearRenderTarget(Graphics->DeviceContext, ResourceType);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, nullptr);

    DrawFade();
    DrawLetterBox(Viewport);

    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FCameraPostProcess::ClearRenderArr()
{
}

void FCameraPostProcess::CreateShader()
{
    // 정점 셰이더 및 입력 레이아웃 생성
    HRESULT hr = ShaderManager->AddVertexShader(VertexShaderKeyLetterBox, ShaderPath, VertexShaderEntryLetterBox);
    if (FAILED(hr))
    {
        return;
    }
    // 픽셀 셰이더 생성
    hr = ShaderManager->AddPixelShader(PixelShaderKeyLetterBox, ShaderPath, PixelShaderEntryLetterBox);
    if (FAILED(hr))
    {
        return;
    }

    // 정점 셰이더 및 입력 레이아웃 생성
    hr = ShaderManager->AddVertexShader(VertexShaderKeyFade, ShaderPath, VertexShaderEntryFade);
    if (FAILED(hr))
    {
        return;
    }
    // 픽셀 셰이더 생성
    hr = ShaderManager->AddPixelShader(PixelShaderKeyFade, ShaderPath, PixelShaderEntryFade);
    if (FAILED(hr))
    {
        return;
    }
}

void FCameraPostProcess::CreateConstantBuffer()
{
    BufferManager->CreateConstantBuffer<FLetterBoxParams>(ConstantBufferKeyLetterBox);
    BufferManager->CreateConstantBuffer<FFadeParams>(ConstantBufferKeyFade);
}

void FCameraPostProcess::ReleaseShader()
{
}

void FCameraPostProcess::ReloadShader()
{
}

void FCameraPostProcess::PrepareRenderStateLetterBox()
{
    // 셰이더 설정
    ShaderManager->SetVertexShader(VertexShaderKeyLetterBox);
    ShaderManager->SetPixelShader(PixelShaderKeyLetterBox);
    Graphics->DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);

    Graphics->DeviceContext->IASetInputLayout(nullptr);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->RSSetState(Graphics->RasterizerSolidBack);
}

void FCameraPostProcess::PrepareRenderStateFade()
{
    // 셰이더 설정
    ShaderManager->SetVertexShader(VertexShaderKeyFade);
    ShaderManager->SetPixelShader(PixelShaderKeyFade);
}

void FCameraPostProcess::DrawLetterBox(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    // 미지정
    if (LetterBoxParams.LetterBoxRatio <= 0.0f)
    {
        return;
    }

    // 현재 스크린 비율 가져옴
    LetterBoxParams.TargetAspectRatio = Viewport->GetViewport()->GetD3DViewport().Width / Viewport->GetViewport()->GetD3DViewport().Height;

    BufferManager->BindConstantBuffer(ConstantBufferKeyLetterBox, 0, EShaderStage::Pixel);
    BufferManager->UpdateConstantBuffer(ConstantBufferKeyLetterBox, LetterBoxParams);


    PrepareRenderStateLetterBox();

    Graphics->DeviceContext->Draw(6, 0);

    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FCameraPostProcess::DrawFade()
{
    // Fade가 필요하지 않으면 리턴
    if (FadeParams.FadeAmount <= 0.0f)
    {
        return;
    }
    BufferManager->BindConstantBuffer(ConstantBufferKeyFade, 0, EShaderStage::Pixel);
    BufferManager->UpdateConstantBuffer(ConstantBufferKeyFade, FadeParams);

    PrepareRenderStateFade();

    Graphics->DeviceContext->Draw(6, 0);

}
