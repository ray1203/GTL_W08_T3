#pragma once

#include "IRenderPass.h"
#include "EngineBaseTypes.h"
#include "Container/Set.h"
#include "Define.h"

class FDXDShaderManager;
class UWorld;
class FEditorViewportClient;

class UPointLightComponent;
class USpotLightComponent;
class UDirectionalLightComponent;
class UAmbientLightComponent;
class FDirectionalShadowMap;
class FSpotLightShadowMap;
class FUpdateLightBufferPass : public IRenderPass
{
public:
    FUpdateLightBufferPass();
    virtual ~FUpdateLightBufferPass();

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    virtual void PrepareRender() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;
    void UpdateLightBuffer() const;

    void SetPointLightShadowMap(FPointLightShadowMap* InPointLightShadowMap);
    void SetDirectionalShadowMap(FDirectionalShadowMap* InDirectionalShadowMap);
    void SetSpotLightShadowMap(FSpotLightShadowMap* InSpotLightShadowMap);

private:
    TArray<USpotLightComponent*> SpotLights;
    TArray<UPointLightComponent*> PointLights;
    TArray<UDirectionalLightComponent*> DirectionalLights;
    TArray<UAmbientLightComponent*> AmbientLights;
    
    FEditorViewportClient* CurrentViewport = nullptr;
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;

    ID3D11Buffer* AmbientBuffer = nullptr;
    ID3D11ShaderResourceView* AmbientSRV = nullptr;
    ID3D11Buffer* DirectionalBuffer = nullptr;
    ID3D11ShaderResourceView* DirectionalSRV = nullptr;
    ID3D11Buffer* PointBuffer = nullptr;
    ID3D11ShaderResourceView* PointSRV = nullptr;
    ID3D11Buffer* SpotBuffer = nullptr;
    ID3D11ShaderResourceView* SpotSRV = nullptr;

    FPointLightShadowMap* PointLightShadowMap = nullptr;
    FDirectionalShadowMap* DirectionalShadowMap = nullptr;
    FSpotLightShadowMap* SpotLightShadowMap = nullptr;
};
