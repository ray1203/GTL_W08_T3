#include "StaticMeshRenderPass.h"

#include <array>

#include "EngineLoop.h"
#include "World/World.h"

#include "RendererHelpers.h"
#include "UnrealClient.h"
#include "Math/JungleMath.h"

#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "Components/StaticMeshComponent.h"

#include "BaseGizmos/GizmoBaseComponent.h"
#include "Engine/EditorEngine.h"

#include "PropertyEditor/ShowFlags.h"

#include "UnrealEd/EditorViewportClient.h"
#include "Shadow/CascadeShadowMap.h"


FStaticMeshRenderPass::FStaticMeshRenderPass()
    : VertexShader(nullptr)
    , PixelShader(nullptr)
    , InputLayout(nullptr)
    , BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FStaticMeshRenderPass::~FStaticMeshRenderPass()
{
    ReleaseShader();
}

void FStaticMeshRenderPass::CreateShader()
{
    // Begin Debug Shaders
    HRESULT hr = ShaderManager->AddPixelShader(L"StaticMeshPixelShaderDepth", L"Shaders/StaticMeshPixelShaderDepth.hlsl", "mainPS");
    if (FAILED(hr))
    {
        return;
    }
    hr = ShaderManager->AddPixelShader(L"StaticMeshPixelShaderWorldNormal", L"Shaders/StaticMeshPixelShaderWorldNormal.hlsl", "mainPS");
    if (FAILED(hr))
    {
        return;
    }
    // End Debug Shaders


#pragma region UberShader
    D3D_SHADER_MACRO DefinesGouraud[] =
    {
        { GOURAUD, "1" },
        { nullptr, nullptr }
    };
    hr = ShaderManager->AddPixelShader(L"GOURAUD_StaticMeshPixelShader", L"Shaders/StaticMeshPixelShader.hlsl", "mainPS", DefinesGouraud);
    if (FAILED(hr))
    {
        return;
    }

    D3D_SHADER_MACRO DefinesLambert[] =
    {
        { LAMBERT, "1" },
        { nullptr, nullptr }
    };
    hr = ShaderManager->AddPixelShader(L"LAMBERT_StaticMeshPixelShader", L"Shaders/StaticMeshPixelShader.hlsl", "mainPS", DefinesLambert);
    if (FAILED(hr))
    {
        return;
    }

    D3D_SHADER_MACRO DefinesBlinnPhong[] =
    {
        { PHONG, "1" },
        { nullptr, nullptr }
    };
    hr = ShaderManager->AddPixelShader(L"PHONG_StaticMeshPixelShader", L"Shaders/StaticMeshPixelShader.hlsl", "mainPS", DefinesBlinnPhong);
    if (FAILED(hr))
    {
        return;
    }

#pragma endregion UberShader

    VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
    LightDepthOnlyVS = ShaderManager->GetVertexShaderByKey(L"LightDepthOnlyVS");
    InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
    InputLayoutLightDepthOnly = ShaderManager->GetInputLayoutByKey(L"LightDepthOnlyVS");

    PixelShader = ShaderManager->GetPixelShaderByKey(L"PHONG_StaticMeshPixelShader");
    DebugDepthShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShaderDepth");
    DebugWorldNormalShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShaderWorldNormal");
}

void FStaticMeshRenderPass::ReleaseShader()
{

}

void FStaticMeshRenderPass::ChangeViewMode(EViewModeIndex ViewModeIndex)
{
    switch (ViewModeIndex)
    {
    case EViewModeIndex::VMI_Lit_Gouraud:
        VertexShader = ShaderManager->GetVertexShaderByKey(L"GOURAUD_StaticMeshVertexShader");
        InputLayout = ShaderManager->GetInputLayoutByKey(L"GOURAUD_StaticMeshVertexShader");
        PixelShader = ShaderManager->GetPixelShaderByKey(L"GOURAUD_StaticMeshPixelShader");
        UpdateLitUnlitConstant(1);
        break;
    case EViewModeIndex::VMI_Lit_Lambert:
        VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
        InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
        PixelShader = ShaderManager->GetPixelShaderByKey(L"LAMBERT_StaticMeshPixelShader");
        UpdateLitUnlitConstant(1);
        break;
    case EViewModeIndex::VMI_Lit_BlinnPhong:
        VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
        InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
        PixelShader = ShaderManager->GetPixelShaderByKey(L"PHONG_StaticMeshPixelShader");
        UpdateLitUnlitConstant(1);
        break;
    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
        VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
        InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
        PixelShader = ShaderManager->GetPixelShaderByKey(L"PHONG_StaticMeshPixelShader");
        UpdateLitUnlitConstant(0);
        break;
    }
}

void FStaticMeshRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;

    CreateShader();
}

void FStaticMeshRenderPass::PrepareRender()
{
    for (const auto iter : TObjectRange<UStaticMeshComponent>())
    {
        if (!Cast<UGizmoBaseComponent>(iter) && iter->GetWorld() == GEngine->ActiveWorld)
        {
            StaticMeshComponents.Add(iter);
        }
    }
}

void FStaticMeshRenderPass::PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    const EViewModeIndex ViewMode = Viewport->GetViewMode();

    if (bIsShadowPass)
    {
        Graphics->DeviceContext->VSSetShader(LightDepthOnlyVS, nullptr, 0);
        Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
        Graphics->DeviceContext->IASetInputLayout(InputLayoutLightDepthOnly);
        Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


        BufferManager->BindConstantBuffer(TEXT("FObjectConstantBuffer"), 12, EShaderStage::Vertex);
        BufferManager->BindConstantBuffer(TEXT("FCameraConstantBuffer"), 13, EShaderStage::Vertex);
    }
    else
    {

        Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
        Graphics->DeviceContext->IASetInputLayout(InputLayout);

        Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        TArray<FString> PSBufferKeys = {
            TEXT("FLightInfoBuffer"),
            TEXT("FMaterialConstants"),
            TEXT("FLitUnlitConstants"),
            TEXT("FSubMeshConstants"),
            TEXT("FTextureConstants")
        };

        BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);

        BufferManager->BindConstantBuffer(TEXT("FLightInfoBuffer"), 0, EShaderStage::Vertex);
        BufferManager->BindConstantBuffer(TEXT("FMaterialConstants"), 1, EShaderStage::Vertex);

    }

    ChangeViewMode(ViewMode);

    // Rasterizer
    if (ViewMode == EViewModeIndex::VMI_Wireframe)
    {
        Graphics->DeviceContext->RSSetState(Graphics->RasterizerWireframeBack);
    }
    else
    {
        Graphics->DeviceContext->RSSetState(Graphics->RasterizerSolidBack);
    }

    // Pixel Shader
    if (ViewMode == EViewModeIndex::VMI_SceneDepth)
    {
        Graphics->DeviceContext->PSSetShader(DebugDepthShader, nullptr, 0);
    }
    else if (ViewMode == EViewModeIndex::VMI_WorldNormal)
    {
        Graphics->DeviceContext->PSSetShader(DebugWorldNormalShader, nullptr, 0);
    }
    else
    {
        Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    }
}

void FStaticMeshRenderPass::UpdateObjectConstant(const FMatrix& WorldMatrix, const FVector4& UUIDColor, bool bIsSelected) const
{
    FObjectConstantBuffer ObjectData = {};
    ObjectData.WorldMatrix = WorldMatrix;
    ObjectData.InverseTransposedWorld = FMatrix::Transpose(FMatrix::Inverse(WorldMatrix));
    ObjectData.UUIDColor = UUIDColor;
    ObjectData.bIsSelected = bIsSelected;

    BufferManager->UpdateConstantBuffer(TEXT("FObjectConstantBuffer"), ObjectData);
}

void FStaticMeshRenderPass::UpdateLitUnlitConstant(int32 isLit) const
{
    FLitUnlitConstants Data;
    Data.bIsLit = isLit;
    BufferManager->UpdateConstantBuffer(TEXT("FLitUnlitConstants"), Data);
}

void FStaticMeshRenderPass::RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData, TArray<FStaticMaterial*> Materials, TArray<UMaterial*> OverrideMaterials, int SelectedSubMeshIndex) const
{
    UINT Stride = sizeof(FStaticMeshVertex);
    UINT Offset = 0;

    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &Stride, &Offset);

    if (RenderData->IndexBuffer)
    {
        Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }

    if (RenderData->MaterialSubsets.Num() == 0)
    {
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
        return;
    }

    for (int SubMeshIndex = 0; SubMeshIndex < RenderData->MaterialSubsets.Num(); SubMeshIndex++)
    {
        uint32 MaterialIndex = RenderData->MaterialSubsets[SubMeshIndex].MaterialIndex;

        FSubMeshConstants SubMeshData = (SubMeshIndex == SelectedSubMeshIndex) ? FSubMeshConstants(true) : FSubMeshConstants(false);

        BufferManager->UpdateConstantBuffer(TEXT("FSubMeshConstants"), SubMeshData);

        if (OverrideMaterials[MaterialIndex] != nullptr)
        {
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, OverrideMaterials[MaterialIndex]->GetMaterialInfo());
        }
        else
        {
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, Materials[MaterialIndex]->Material->GetMaterialInfo());
        }

        uint32 StartIndex = RenderData->MaterialSubsets[SubMeshIndex].IndexStart;
        uint32 IndexCount = RenderData->MaterialSubsets[SubMeshIndex].IndexCount;
        Graphics->DeviceContext->DrawIndexed(IndexCount, StartIndex, 0);
    }
}

void FStaticMeshRenderPass::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const
{
    UINT Stride = sizeof(FStaticMeshVertex);
    UINT Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &Offset);
    Graphics->DeviceContext->Draw(numVertices, 0);
}

void FStaticMeshRenderPass::RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const
{
    UINT Stride = sizeof(FStaticMeshVertex);
    UINT Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &Offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void FStaticMeshRenderPass::Render(
    const std::shared_ptr<FEditorViewportClient>& Viewport,
    FCascadeShadowMap* CascadeShadowMap,
    bool IsShadow)
{
    bIsShadowPass = IsShadow;
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(EResourceType::ERT_Scene);
    //ToDo: 리팩토링 필요
    if (bIsShadowPass)
    {
        // 0) 이전 frame에 바인딩된 SRV 해제
        ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);

        // 1) 그림자 맵 크기에 맞춰 리사이즈 & 뷰포트 설정
        CascadeShadowMap->ResizeTexture(Viewport.get());
    
        //// 2) Depth-only 스텐실 상태 생성/바인딩
        //D3D11_DEPTH_STENCIL_DESC dsDesc{};
        //dsDesc.DepthEnable = TRUE;
        //dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        //dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        //ID3D11DepthStencilState* dsState = nullptr;
        //Graphics->Device->CreateDepthStencilState(&dsDesc, &dsState);
        //Graphics->DeviceContext->OMSetDepthStencilState(dsState, 0);

        // 3) DSV만 바인딩 후 클리어
        Graphics->DeviceContext->OMSetRenderTargets(
            0, nullptr,
            CascadeShadowMap->ShadowDSV
        );
        Graphics->DeviceContext->ClearDepthStencilView(
            CascadeShadowMap->ShadowDSV,
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f, 0);

        // 4) Shadow-pass 전용 셰이더 및 상수/버퍼 설정
        PrepareRenderState(Viewport);
        CascadeShadowMap->PrepareRender(Viewport.get());

        // 5) 모든 StaticMeshComponent에 대해 Depth-only DrawIndexed
        for (UStaticMeshComponent* Comp : StaticMeshComponents)
        {
            if (!Comp || !Comp->GetStaticMesh()) continue;
            auto* RD = Comp->GetStaticMesh()->GetRenderData();
            if (!RD) continue;

            // 세팅: 월드 매트릭스만 업데이트
            UpdateObjectConstant(Comp->GetWorldMatrix(), Comp->EncodeUUID() / 255.0f, false);

            // 실제로 Depth-only 드로우
            RenderPrimitive(RD,
                Comp->GetStaticMesh()->GetMaterials(),
                Comp->GetOverrideMaterials(),
                Comp->GetselectedSubMeshIndex());
        }

        // 6) DSV 언바인드 및 상태 해제
        Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
        //dsState->Release();

        // 7) ImGui로 Depth SRV 시각화
        ImGui::Image((ImTextureID)CascadeShadowMap->ShadowSRV,
            ImVec2((float)512, (float)512));
    }
    else
    {
        // --- 기존 메인 패스 로직 유지 ---
        Graphics->DeviceContext->OMSetRenderTargets(
            1, &RenderTargetRHI->RTV,
            ViewportResource->GetDepthStencilView()
        );
        ViewportResource->ClearRenderTarget(Graphics->DeviceContext, EResourceType::ERT_Scene);
        Graphics->DeviceContext->ClearDepthStencilView(
            ViewportResource->GetDepthStencilView(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f, 0);

        PrepareRenderState(Viewport);

        for (UStaticMeshComponent* Comp : StaticMeshComponents)
        {
            if (!Comp || !Comp->GetStaticMesh()) continue;
            auto* RD = Comp->GetStaticMesh()->GetRenderData();
            if (!RD) continue;

            UpdateObjectConstant(Comp->GetWorldMatrix(), Comp->EncodeUUID() / 255.0f,
                Cast<UEditorEngine>(GEngine)->GetSelectedActor() == Comp->GetOwner());
            RenderPrimitive(RD,
                Comp->GetStaticMesh()->GetMaterials(),
                Comp->GetOverrideMaterials(),
                Comp->GetselectedSubMeshIndex());

            if (Viewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
            {
                FEngineLoop::PrimitiveDrawBatch.AddAABBToBatch(
                    Comp->GetBoundingBox(),
                    Comp->GetWorldLocation(),
                    Comp->GetWorldMatrix());
            }
        }

    }
        Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FStaticMeshRenderPass::ClearRenderArr()
{
    StaticMeshComponents.Empty();
}

