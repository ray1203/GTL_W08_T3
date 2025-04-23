#pragma once
#include "Define.h"
#include "Math/Matrix.h"
#include "UnrealEd/EditorViewportClient.h"

class FDXDBufferManager;
class FGraphicsDevice;
class FDXDShaderManager;
class USpotLightComponent;

struct FSpotLightShadowResource 
{
    ID3D11Texture2D* DepthStencilBuffer = nullptr;
    ID3D11DepthStencilView* ShadowDSV = nullptr;
    ID3D11ShaderResourceView* ShadowSRV = nullptr;
    FMatrix SpotLightViewProjMatrix;
};

class FSpotLightShadowMap
{
public:
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager);
    void UpdateSpotLightViewProjMatrices(int index, const FSpotLightInfo& Info);
    void PrepareRender();
    void RenderShadowMap();
    void ClearRenderArr();

    void SetShadowResource(int tStart);
    void SetShadowSampler(int sStart);

    ID3D11ShaderResourceView* GetShadowSRV(int index);
    ID3D11ShaderResourceView* GetShadowViewSRV();
    
    FMatrix GetViewProjMatrix(int index);

    void RenderLinearDepth(int index);

    void AddSpotLightResource(int num);
    void DeleteSpotLightResource(int num);

private:
    FDXDBufferManager* BufferManager = nullptr;
    FGraphicsDevice* Graphics = nullptr;
    FDXDShaderManager* ShaderManager = nullptr;

    ID3D11VertexShader* DepthVS = nullptr;
    ID3D11InputLayout* DepthIL = nullptr;

    TArray<USpotLightComponent*> SpotLights;

    TArray<FSpotLightShadowResource> SpotLightShadowResources;
    int prevSpotNum = 0;

    ID3D11SamplerState* ShadowSampler = nullptr;
    uint32 ShadowMapSize = 1024;

    // Begin ImGui Debug
    ID3D11Texture2D* DepthLinearBuffer = { nullptr };
    ID3D11ShaderResourceView* ShadowViewSRV = { nullptr };
    ID3D11RenderTargetView* ShadowViewRTV = { nullptr };
    ID3D11SamplerState* LinearSampler = nullptr;
    ID3D11VertexShader* FullscreenVS = nullptr;
    ID3D11InputLayout* FullscreenIL = nullptr;
    ID3D11PixelShader* DepthVisualizePS = nullptr;
    // End ImGui Debug
};
