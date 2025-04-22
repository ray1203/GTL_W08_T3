#include "Define.h"
#include "UObject/Casts.h"
#include "UpdateLightBufferPass.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Components/Light/AmbientLightComponent.h"
#include "Engine/EditorEngine.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"
#include "Renderer/Shadow/PointLightShadowMap.h"
#include "Renderer/Shadow/DirectionalShadowMap.h"
#include "Renderer/Shadow/SpotLightShadowMap.h"

//------------------------------------------------------------------------------
// 생성자/소멸자
//------------------------------------------------------------------------------
FUpdateLightBufferPass::FUpdateLightBufferPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FUpdateLightBufferPass::~FUpdateLightBufferPass()
{
}

void FUpdateLightBufferPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
}

void FUpdateLightBufferPass::PrepareRender()
{
    for (const auto iter : TObjectRange<ULightComponentBase>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
            {
                PointLights.Add(PointLight);
            }
            else if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(iter))
            {
                SpotLights.Add(SpotLight);
            }
            else if (UDirectionalLightComponent* DirectionalLight = Cast<UDirectionalLightComponent>(iter))
            {
                DirectionalLights.Add(DirectionalLight);
            }
            // Begin Test
            else if (UAmbientLightComponent* AmbientLight = Cast<UAmbientLightComponent>(iter))
            {
                AmbientLights.Add(AmbientLight);
            }
            // End Test
        }
    }
}

void FUpdateLightBufferPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    CurrentViewport = Viewport.get();
    UpdateLightBuffer();

}

void FUpdateLightBufferPass::ClearRenderArr()
{
    PointLights.Empty();
    SpotLights.Empty();
    DirectionalLights.Empty();
    AmbientLights.Empty();
}


void FUpdateLightBufferPass::UpdateLightBuffer() const
{
    FLightInfoBuffer LightBufferData = {};

    int DirectionalLightsCount=0;
    int PointLightsCount=0;
    int SpotLightsCount=0;
    int AmbientLightsCount=0;

    for (auto Light : SpotLights)
    {
        if (SpotLightsCount < MAX_SPOT_LIGHT)
        {
            LightBufferData.SpotLights[SpotLightsCount] = Light->GetSpotLightInfo();
            LightBufferData.SpotLights[SpotLightsCount].Position = Light->GetWorldLocation();
            LightBufferData.SpotLights[SpotLightsCount].Direction = Light->GetDirection();
            LightBufferData.SpotLights[SpotLightsCount].SpotLightViewProj = SpotLightShadowMap->GetViewProjMatrix(SpotLightsCount);
            SpotLightsCount++;
        }
    }

    for (auto Light : PointLights)
    {
        if (PointLightsCount < MAX_POINT_LIGHT)
        {
            LightBufferData.PointLights[PointLightsCount] = Light->GetPointLightInfo();
            LightBufferData.PointLights[PointLightsCount].Position = Light->GetWorldLocation();
            for (int face = 0; face < FPointLightShadowMap::faceNum; face++) {
                LightBufferData.PointLights[PointLightsCount].PointLightViewProj[face] = PointLightShadowMap->GetViewProjMatrix(PointLightsCount, face);
            }
            PointLightsCount++;
        }
    }

    for (auto Light : DirectionalLights)
    {
        if (DirectionalLightsCount < MAX_DIRECTIONAL_LIGHT)
        {
            LightBufferData.Directional[DirectionalLightsCount] = Light->GetDirectionalLightInfo();
            LightBufferData.Directional[DirectionalLightsCount].Direction = Light->GetDirection();
            LightBufferData.Directional[DirectionalLightsCount].LightView = DirectionalShadowMap->GetDirectionalView(DirectionalLightsCount);
            LightBufferData.Directional[DirectionalLightsCount].LightProj = DirectionalShadowMap->GetDirectionalProj(DirectionalLightsCount);

            DirectionalLightsCount++;
        }
    }

    for (auto Light : AmbientLights)
    {
        if (AmbientLightsCount < MAX_DIRECTIONAL_LIGHT)
        {
            LightBufferData.Ambient[AmbientLightsCount] = Light->GetAmbientLightInfo();
            LightBufferData.Ambient[AmbientLightsCount].AmbientColor = Light->GetLightColor();
            AmbientLightsCount++;
        }
    }
    
    LightBufferData.DirectionalLightsCount = DirectionalLightsCount;
    LightBufferData.PointLightsCount = PointLightsCount;
    LightBufferData.SpotLightsCount = SpotLightsCount;
    LightBufferData.AmbientLightsCount = AmbientLightsCount;

    BufferManager->UpdateConstantBuffer(TEXT("FLightInfoBuffer"), LightBufferData);
     
}

void FUpdateLightBufferPass::SetPointLightShadowMap(FPointLightShadowMap* InPointLightShadowMap)
{
    PointLightShadowMap = InPointLightShadowMap;
}

void FUpdateLightBufferPass::SetDirectionalShadowMap(FDirectionalShadowMap* InDirectionalShadowMap)
{
    DirectionalShadowMap = InDirectionalShadowMap;
}

void FUpdateLightBufferPass::SetSpotLightShadowMap(FSpotLightShadowMap* InSpotLightShadowMap)
{
    SpotLightShadowMap = InSpotLightShadowMap;
}
