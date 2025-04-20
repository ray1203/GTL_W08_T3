#pragma once
#include "Define.h"
#include "Math/Matrix.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Components/Light/DirectionalLightComponent.h"

class FDXDBufferManager;
class FGraphicsDevice;
class FDXDShaderManager;

struct FCascade
{
    FMatrix ViewProjMatrix;
    float SplitDepth;
    D3D11_VIEWPORT ShadowViewport;
};
namespace
{

    FVector ComputeFrustumCenter(const TArray<FVector>& FrustumCorners)
    {
        FVector center = FVector::ZeroVector;
        for (const FVector& corner : FrustumCorners)
        {
            center += corner;
        }
        return center / FrustumCorners.Num();
    }
};

class FCascadeShadowMap
{
public:
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager);
    void UpdateCascadeViewProjMatrices(FEditorViewportClient& ViewCamera, const FVector& LightDir);
    void PrepareRender();
    void ResizeTexture(FEditorViewportClient* ViewCamera);
    void PrepareRender(FEditorViewportClient* Viewport);
    void Render(FEditorViewportClient* Viewport);
    const TArray<FCascade>& GetCascades() const;

    ID3D11Texture2D* DepthStencilTexture = nullptr;

    ID3D11DepthStencilView* ShadowDSV = nullptr;
    ID3D11ShaderResourceView* ShadowSRV = nullptr;
    ID3D11SamplerState* ShadowSampler = nullptr;

private:

    TArray<UDirectionalLightComponent*> DirectionalLights;

    float Width, Height;
    FDXDBufferManager* BufferManager = nullptr;
    FGraphicsDevice* Graphics = nullptr;
    FDXDShaderManager* ShaderManager = nullptr;

    FMatrix CascadeViewProjMatrix;
    TArray<FCascade> Cascades;
};
