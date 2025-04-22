#pragma once
#include "Define.h"
#include "Math/Matrix.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Components/Light/DirectionalLightComponent.h"

class FDXDBufferManager;
class FGraphicsDevice;
class FDXDShaderManager;
class UStaticMeshComponent;

class FDXDShaderManager;
class UWorld;
class UMaterial;
class FEditorViewportClient;
class UStaticMeshComponent;
struct FStaticMaterial;

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

class FDirectionalShadowMap
{
public:
    ~FDirectionalShadowMap();
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager);
    void CreateComparisonSampler();
    void CreateShadowRasterizer();
    void CreateDepthStencilState();
    void LoadShadowShaders();
    void UpdateViewProjMatrices(FEditorViewportClient& ViewCamera, const FVector& LightDir);
    void ChangeViewportSize();
    void CreateDepthTexture();
    void PrepareRender(FEditorViewportClient* Viewport);
    void PrepareRenderState(FEditorViewportClient* Viewport);
    void CollectStaticMeshes();
    void CollectDirectionalLights();
    void UpdateObjectConstant(const FMatrix& WorldMatrix, const FVector4& UUIDColor, bool bIsSelected) const;
    void RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData, TArray<FStaticMaterial*> Materials, TArray<UMaterial*> OverrideMaterials, int SelectedSubMeshIndex) const;
    void Render(FEditorViewportClient* Viewport);
    
    FMatrix GetDireictionalView(int index);
    FMatrix GetDirectionalProj(int index);

    ID3D11Texture2D* DepthStencilTexture = nullptr;

    ID3D11DepthStencilView* ShadowDSV = nullptr;
    ID3D11ShaderResourceView* ShadowSRV = nullptr;
    ID3D11SamplerState* ShadowSampler = nullptr;

    float ShadowResolution = 4096;
private:
    ID3D11RasterizerState* ShadowRasterizer;
    ID3D11DepthStencilState* dsState;
    TArray<UDirectionalLightComponent*> DirectionalLights;
    TArray<UStaticMeshComponent*> StaticMeshComponents;

    FDXDBufferManager* BufferManager = nullptr;
    FGraphicsDevice* Graphics = nullptr;
    FDXDShaderManager* ShaderManager = nullptr;

    ID3D11VertexShader* LightDepthOnlyVS;
    ID3D11InputLayout* InputLayoutLightDepthOnly;

    TArray<FMatrix> DirectionalViews;
    TArray<FMatrix> DirectionalProjs;
};
