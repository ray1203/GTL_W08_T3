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

    ID3D11Texture2D* VSMTexture = nullptr;         
    ID3D11RenderTargetView* VSMRTV = nullptr;      
    ID3D11ShaderResourceView* VSMSRV = nullptr;    
    ID3D11Texture2D* VSMDepthBuffer = nullptr;     
    ID3D11DepthStencilView* VSMDepthDSV = nullptr; 

    FMatrix SpotLightViewProjMatrix;
};

class FSpotLightShadowMap
{
public:
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager, EShadowFilter InShadowFilter = EShadowFilter::ESF_VSM);
    void InitializeDebugVisualizationResources();

    void UpdateSpotLightViewProjMatrices(int index, const FSpotLightInfo& Info);
    void PrepareRender();
    void RenderShadowMap();
    void ClearRenderArr();

    void SetShadowResource(int tStart);
    void SetShadowResources(int pcfTextureSlotStart, int vsmTextureSlotStart);
    void SetShadowSampler(int sStart);
    void SetShadowFilterSampler(int sStart);


    ID3D11ShaderResourceView* GetShadowSRV(int index);
    ID3D11ShaderResourceView* GetShadowDebugSRV();
    
    FMatrix GetViewProjMatrix(int index);

    void RenderLinearDepth();

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
    ID3D11Texture2D* DebugDepthLinearBuffer = { nullptr };
    ID3D11ShaderResourceView* ShadowDebugSRV = { nullptr };
    ID3D11RenderTargetView* ShadowDebugRTV = { nullptr };
    ID3D11SamplerState* DebugSampler = nullptr;
    ID3D11VertexShader* FullscreenVS = nullptr;
    ID3D11InputLayout* FullscreenIL = nullptr;
    ID3D11PixelShader* DepthVisualizePS = nullptr;
    // End ImGui Debug

    EShadowFilter ShadowFilter;
    ID3D11SamplerState* ShadowSamplerVSM = nullptr;
    ID3D11PixelShader* VSMGenerationPS = nullptr;
};
