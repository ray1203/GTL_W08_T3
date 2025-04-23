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

    auto CreateSB = [&](UINT stride, UINT maxElements,
        ID3D11Buffer*& outBuf,
        ID3D11ShaderResourceView*& outSRV)
        {
            D3D11_BUFFER_DESC bd = {};
            bd.Usage = D3D11_USAGE_DYNAMIC;
            bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            bd.StructureByteStride = stride;
            bd.ByteWidth = stride * maxElements;
            Graphics->Device->CreateBuffer(&bd, nullptr, &outBuf);

            D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
            srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            srvd.Format = DXGI_FORMAT_UNKNOWN;
            srvd.Buffer.FirstElement = 0;
            srvd.Buffer.NumElements = maxElements;
            Graphics->Device->CreateShaderResourceView(outBuf, &srvd, &outSRV);
        };

    // Ambient
    CreateSB(sizeof(FAmbientLightInfo), MAX_AMBIENT_LIGHT, AmbientBuffer, AmbientSRV);
    // Directional
    CreateSB(sizeof(FDirectionalLightInfo), MAX_DIRECTIONAL_LIGHT, DirectionalBuffer, DirectionalSRV);
    // Point
    CreateSB(sizeof(FPointLightInfo), MAX_POINT_LIGHT, PointBuffer, PointSRV);
    // Spot
    CreateSB(sizeof(FSpotLightInfo), MAX_SPOT_LIGHT, SpotBuffer, SpotSRV);

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
    FLightInfoBuffer LightBufferData;

    // --- 1) CPU 쪽 임시 Array에 Light 정보 채우기 ---
    TArray<FAmbientLightInfo>    ambientData;    ambientData.Reserve(MAX_AMBIENT_LIGHT);
    TArray<FDirectionalLightInfo> directionalData; directionalData.Reserve(MAX_DIRECTIONAL_LIGHT);
    TArray<FPointLightInfo>       pointData;       pointData.Reserve(MAX_POINT_LIGHT);
    TArray<FSpotLightInfo>        spotData;        spotData.Reserve(MAX_SPOT_LIGHT);

    UINT AmbientLightsCount = 0;
    UINT DirectionalLightsCount = 0;
    UINT PointLightsCount = 0;
    UINT SpotLightsCount = 0;

    for (auto* L : AmbientLights)
    {
        if (AmbientLightsCount >= MAX_AMBIENT_LIGHT) break;
        auto info = L->GetAmbientLightInfo();
        info.AmbientColor = L->GetLightColor();
        ambientData.Add(info);
        ++AmbientLightsCount;
    }
    for (auto* L : DirectionalLights)
    {
        if (DirectionalLightsCount >= MAX_DIRECTIONAL_LIGHT) break;
        auto info = L->GetDirectionalLightInfo();
        info.Direction = L->GetDirection();
        info.LightView = DirectionalShadowMap->GetDirectionalView(DirectionalLightsCount);
        info.LightProj = DirectionalShadowMap->GetDirectionalProj(DirectionalLightsCount);
        directionalData.Add(info);
        ++DirectionalLightsCount;
    }
    for (auto* L : PointLights)
    {
        if (PointLightsCount >= MAX_POINT_LIGHT) break;
        auto info = L->GetPointLightInfo();
        info.Position = L->GetWorldLocation();
        for (int f = 0; f < FPointLightShadowMap::faceNum; ++f)
            info.PointLightViewProj[f] =
            PointLightShadowMap->GetViewProjMatrix(PointLightsCount, f);
        pointData.Add(info);
        ++PointLightsCount;
    }
    for (auto* L : SpotLights)
    {
        if (SpotLightsCount >= MAX_SPOT_LIGHT) break;
        auto info = L->GetSpotLightInfo();
        info.Position = L->GetWorldLocation();
        info.Direction = L->GetDirection();
        info.SpotLightViewProj =
            SpotLightShadowMap->GetViewProjMatrix(SpotLightsCount);
        spotData.Add(info);
        ++SpotLightsCount;
    }

    // ---------------------------------------------------
    // 2) Map / Unmap으로 Structured Buffer에 복사
    // ---------------------------------------------------
    auto UpdateSB = [&](ID3D11Buffer* buf, const void* src, UINT64 byteSize)
        {
            D3D11_MAPPED_SUBRESOURCE M = {};
            Graphics->DeviceContext->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &M);
            memcpy(M.pData, src, byteSize);
            Graphics->DeviceContext->Unmap(buf, 0);
        };

    if (AmbientLightsCount)
        UpdateSB(AmbientBuffer, ambientData.GetData(), sizeof(FAmbientLightInfo) * AmbientLightsCount);
    if (DirectionalLightsCount)
        UpdateSB(DirectionalBuffer, directionalData.GetData(), sizeof(FDirectionalLightInfo) * DirectionalLightsCount);
    if (PointLightsCount)
        UpdateSB(PointBuffer, pointData.GetData(), sizeof(FPointLightInfo) * PointLightsCount);
    if (SpotLightsCount)
        UpdateSB(SpotBuffer, spotData.GetData(), sizeof(FSpotLightInfo) * SpotLightsCount);
    
    LightBufferData.DirectionalLightsCount = DirectionalLightsCount;
    LightBufferData.PointLightsCount = PointLightsCount;
    LightBufferData.SpotLightsCount = SpotLightsCount;
    LightBufferData.AmbientLightsCount = AmbientLightsCount;

    BufferManager->UpdateConstantBuffer(TEXT("FLightInfoBuffer"), LightBufferData);
    
    Graphics->DeviceContext->PSSetShaderResources(60, 1, &AmbientSRV);
    Graphics->DeviceContext->PSSetShaderResources(61, 1, &DirectionalSRV);
    Graphics->DeviceContext->PSSetShaderResources(62, 1, &PointSRV);
    Graphics->DeviceContext->PSSetShaderResources(63, 1, &SpotSRV);
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
