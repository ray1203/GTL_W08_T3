#pragma once
#include "Define.h"
#include "Math/Matrix.h"
#include "UnrealEd/EditorViewportClient.h"

class FDXDBufferManager;
class FGraphicsDevice;
class FDXDShaderManager;
class USpotLightComponent;

class FSpotLightShadowMap
{
public:
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager);
    void UpdateSpotLightViewProjMatrices(const FSpotLightInfo& Info);
    void PrepareRender();
    void RenderShadowMap();
    void UpdateConstantBuffer();
    void ClearRenderArr();

    void SetShadowResource(int tStart);
    void SetShadowSampler(int sStart);

    ID3D11ShaderResourceView* GetShadowSRV();

private:
    FDXDBufferManager* BufferManager = nullptr;
    FGraphicsDevice* Graphics = nullptr;
    FDXDShaderManager* ShaderManager = nullptr;

    ID3D11VertexShader* DepthVS = nullptr;
    ID3D11InputLayout* DepthIL = nullptr;

    TArray<USpotLightComponent*> SpotLights;

    ID3D11Texture2D* DepthStencilBuffer = nullptr;
    ID3D11DepthStencilView* ShadowDSV = nullptr;
    ID3D11ShaderResourceView* ShadowSRV = nullptr;
    ID3D11SamplerState* ShadowSampler = nullptr;

    uint32 ShadowMapSize = 1024;
    FMatrix SpotLightViewProjMatrix = {};

};
