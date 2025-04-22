#pragma once
#include "Define.h"
#include "Math/Matrix.h"
#include "UnrealEd/EditorViewportClient.h"

class FDXDBufferManager;
class FGraphicsDevice;
class FDXDShaderManager;
class UPointLightComponent;

struct FPointLightShadowCube 
{
    ID3D11Texture2D* DepthCube = nullptr;
    ID3D11DepthStencilView* ShadowDSV[6] = { nullptr };
    ID3D11ShaderResourceView* ShadowCubeSRV = nullptr;
    FMatrix PointLightViewProjMatrix[6];
};

class FPointLightShadowMap
{
public:
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager);
    void UpdatePointLightViewProjMatrices(int index, const FVector& pointLightPos, const float lightRadius);
    void PrepareRender();
    void RenderShadowMap();
    void ClearRenderArr();

    void SetShadowResource(int tStart);
    void SetShadowSampler(int sStart);

    void AddPointLightShadowCube(int num);
    void DeletePointLightShadowCube(int num);

    void RenderLinearDepth();

    TArray<ID3D11ShaderResourceView*> GetShadowViewSRVArray();

    TArray<FVector> GetDirectionArray();
    TArray<FVector> GetUpArray();

    FMatrix GetViewProjMatrix(int index, int face); // Constanc 업데이트를 LightBufferPass를 통해 진행

private:
    FDXDBufferManager* BufferManager = nullptr;
    FGraphicsDevice* Graphics = nullptr;
    FDXDShaderManager* ShaderManager = nullptr;

    ID3D11VertexShader* DepthVS = nullptr;
    ID3D11InputLayout* DepthIL = nullptr;

    TArray<UPointLightComponent*> PointLights;

    TArray<FPointLightShadowCube> PointLightShadowCubes;
    int prevShadowCubeNum = 0;

    ID3D11SamplerState* ShadowSampler = nullptr;

    // 시각화 하는 용의 Buffer PointLight가 여러개 이더라도
    // 6개만 있어도 됨
    ID3D11Texture2D* DepthLinearBuffer[6] = { nullptr };
    ID3D11ShaderResourceView* ShadowViewSRV[6] = { nullptr };
    ID3D11RenderTargetView* ShadowViewRTV[6] = { nullptr };
    ID3D11SamplerState* LinearSampler = nullptr;

    ID3D11VertexShader* FullscreenVS = nullptr;
    ID3D11InputLayout* FullscreenIL = nullptr;

    ID3D11PixelShader* DepthVisualizePS = nullptr;

    uint32 ShadowMapSize = 1024;

public:
    static const uint32 faceNum = 6;

private:

    const FVector Directions[6] = {
        FVector(1,0,0), FVector(-1,0,0),
        FVector(0,1,0), FVector(0,-1,0),
        FVector(0,0,1), FVector(0,0,-1)
    };
    const FVector Ups[6] = {
        FVector(0, 1, 0), FVector(0,1,0),
        FVector(0,0,-1), FVector(0,0,1),
        FVector(0,1,0), FVector(0,1,0)
    };
};
