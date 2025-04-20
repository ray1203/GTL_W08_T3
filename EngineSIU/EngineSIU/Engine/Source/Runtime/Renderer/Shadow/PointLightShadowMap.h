#pragma once
#include "Define.h"
#include "Math/Matrix.h"
#include "UnrealEd/EditorViewportClient.h"

class FDXDBufferManager;
class FGraphicsDevice;
class FDXDShaderManager;
class UPointLightComponent;

class FPointLightShadowMap
{
public:
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager);
    void UpdatePointLightViewProjMatrices(const FVector& pointLightPos, const float lightRadius);
    void PrepareRender();
    void RenderShadowMap();
    void UpdateConstantBuffer();
    void ClearRenderArr();

    void SetShadowResource(int tStart);
    void SetShadowSampler(int sStart);

    TArray<ID3D11ShaderResourceView*> GetShadowSRVArray();
    TArray<FVector> GetDirectionArray();
    TArray<FVector> GetUpArray();

private:
    FDXDBufferManager* BufferManager = nullptr;
    FGraphicsDevice* Graphics = nullptr;
    FDXDShaderManager* ShaderManager = nullptr;

    ID3D11VertexShader* DepthVS = nullptr;
    ID3D11InputLayout* DepthIL = nullptr;

    TArray<UPointLightComponent*> PointLights;

    // 일단 한 Point Light만 만들어보기
    // TODO 여러 개의 Point Light 가능하도록 변경
    // 아래를 구조체로 다루고 TArray로 동적 관리하도록 해야할듯
    ID3D11Texture2D* DepthStencilBuffer[6] = { nullptr };
    ID3D11DepthStencilView* ShadowDSV[6] = { nullptr };
    ID3D11ShaderResourceView* ShadowSRV[6] = { nullptr };
    ID3D11SamplerState* ShadowSampler = nullptr;

    uint32 ShadowMapSize = 1024;
    const uint32 faceNum = 6;

    FMatrix PointLightViewProjMatrix[6];

    const FVector Directions[6] = {
        FVector(1,0,0), FVector(-1,0,0),
        FVector(0,1,0), FVector(0,-1,0),
        FVector(0,0,1), FVector(0,0,-1)
    };
    const FVector Ups[6] = {
        FVector(0,0,1), FVector(0,0,1),
        FVector(0,0,1), FVector(0,0,1),
        FVector(0,1,0), FVector(0,1,0)
    };
};
