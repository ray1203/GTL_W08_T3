#pragma once
#include "IRenderPass.h"

#include "Container/String.h"
#include "UnrealClient.h"

class FCameraPostProcess :
    public IRenderPass
{
private:
    struct alignas(16) FLetterBoxParams
    {
        FLinearColor BoxColor = FLinearColor(0,0,0,1); // 박스 색상
        float TargetAspectRatio = 0.0f; // 타겟 비율
        float LetterBoxRatio = 1.0f; // 비율
    } LetterBoxParams;

    struct alignas(16) FFadeParams
    {
        FLinearColor FadeColor = FLinearColor(1,0,0,1); // fade 색상
        float FadeAmount = 0.0f; // fade 비율
        FVector Pad;
    } FadeParams;

public:
    FCameraPostProcess();
    virtual ~FCameraPostProcess();

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;

    virtual void PrepareRender() override;

    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

    virtual void ClearRenderArr() override;

    void SetLetterBox(float Ratio, const FLinearColor& Color)
    {
        LetterBoxParams.TargetAspectRatio = Ratio;
        LetterBoxParams.BoxColor = Color;
    }

private:
    void CreateShader();
    void CreateConstantBuffer();

    void ReleaseShader();
    
    void ReloadShader();

    void PrepareRenderStateLetterBox();
    void PrepareRenderStateFade();
    
private:
    void DrawLetterBox(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void DrawFade();

private:
    FGraphicsDevice* Graphics = nullptr;
    FDXDBufferManager* BufferManager = nullptr;
    FDXDShaderManager* ShaderManager = nullptr;

    FRenderTargetRHI RenderTarget;

    const FWString ShaderPath = L"Shaders/CameraPostProcess.hlsl";

    const FWString VertexShaderKeyLetterBox = L"CameraPostProcessLetterBox";
    const FWString PixelShaderKeyLetterBox = L"CameraPostProcessLetterBox";
    const char* VertexShaderEntryLetterBox = "ScreenQuadVS";
    const char* PixelShaderEntryLetterBox = "letterBoxPS";

    const FWString VertexShaderKeyFade = L"CameraPostProcessFade";
    const FWString PixelShaderKeyFade = L"CameraPostProcessFade";
    const char* VertexShaderEntryFade = "ScreenQuadVS";
    const char* PixelShaderEntryFade = "fadePS";

    const FString ConstantBufferKeyLetterBox = TEXT("FConstantBufferCameraPostProcessLetterBox");
    const FString ConstantBufferKeyFade = TEXT("FConstantBufferCameraPostProcessFade");
};

