#include "EditorRenderPass.h"

#include <D3D11RHI/DXDShaderManager.h>
#include "EngineLoop.h" // GEngineLoop
#include "Runtime/CoreUObject/UObject/Casts.h"
#include "Runtime/Engine/Classes/Engine/Engine.h" // GEngine

#include "UnrealClient.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/Classes/Actors/Player.h"
#include "Engine/Classes/Components/HeightFogComponent.h"
#include "Engine/Classes/Components/Light/AmbientLightComponent.h"
#include "Engine/Classes/Components/Light/DirectionalLightComponent.h"
#include "Engine/Classes/Components/Light/LightComponent.h"
#include "Engine/Classes/Components/Light/PointLightComponent.h"
#include "Engine/Classes/Components/Light/SpotLightComponent.h"
#include "Runtime/Engine/World/World.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/UObjectIterator.h"

#include "PropertyEditor/ShowFlags.h"
#include "Engine/EditorEngine.h"

void FEditorRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
    
    CreateShaders();
    CreateBuffers();
    CreateConstantBuffers();
}

void FEditorRenderPass::Release()
{
    ReleaseShaders();
}

void FEditorRenderPass::CreateShaders()
{
    D3D11_INPUT_ELEMENT_DESC layoutGizmo[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    D3D11_INPUT_ELEMENT_DESC layoutPosOnly[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    // Axis
    ShaderManager->AddVertexShaderAndInputLayout(AxisKeyW, L"Shaders/EditorShader.hlsl", "axisVS", layoutPosOnly, ARRAYSIZE(layoutPosOnly));
    ShaderManager->AddPixelShader(AxisKeyW, L"Shaders/EditorShader.hlsl", "axisPS");
 
    // Arrow
    ShaderManager->AddVertexShaderAndInputLayout(ArrowKeyW, L"Shaders/EditorShader.hlsl", "arrowVS", layoutGizmo, ARRAYSIZE(layoutGizmo));
    ShaderManager->AddPixelShader(ArrowKeyW, L"Shaders/EditorShader.hlsl", "arrowPS");

    // Sphere
    ShaderManager->AddVertexShaderAndInputLayout(SphereKeyW, L"Shaders/EditorShader.hlsl", "sphereVS", layoutPosOnly, ARRAYSIZE(layoutPosOnly));
    ShaderManager->AddPixelShader(SphereKeyW, L"Shaders/EditorShader.hlsl", "spherePS");

    // Cone
    ShaderManager->AddVertexShader(ConeKeyW, L"Shaders/EditorShader.hlsl", "coneVS");
    ShaderManager->AddPixelShader(ConeKeyW, L"Shaders/EditorShader.hlsl", "conePS");

    // AABB
    ShaderManager->AddVertexShaderAndInputLayout(AABBKeyW, L"Shaders/EditorShader.hlsl", "aabbVS", layoutPosOnly, ARRAYSIZE(layoutPosOnly));
    ShaderManager->AddPixelShader(AABBKeyW, L"Shaders/EditorShader.hlsl", "aabbPS");

    // Grid
    ShaderManager->AddVertexShader(GridKeyW, L"Shaders/EditorShader.hlsl", "gridVS");
    ShaderManager->AddPixelShader(GridKeyW, L"Shaders/EditorShader.hlsl", "gridPS");

    // Icon
    ShaderManager->AddVertexShader(IconKeyW, L"Shaders/EditorShader.hlsl", "iconVS");
    ShaderManager->AddPixelShader(IconKeyW, L"Shaders/EditorShader.hlsl", "iconPS");

    //auto AddShaderSet = [&](const std::wstring& keyPrefix, const std::string& vsEntry, const std::string& psEntry,
    //    D3D11_INPUT_ELEMENT_DESC* layout, uint32_t layoutSize,
    //    /*D3D11_PRIMITIVE_TOPOLOGY topology, FShaderResource& target*/)
    //    {
    //        ShaderManager->AddVertexShaderAndInputLayout(keyPrefix + L"VS", L"Shaders/EditorShader.hlsl", vsEntry, layout, layoutSize);
    //        ShaderManager->AddPixelShader(keyPrefix + L"PS", L"Shaders/EditorShader.hlsl", psEntry);
    //        //target.Vertex = ShaderManager->GetVertexShaderByKey(keyPrefix + L"VS");
    //        //target.Pixel = ShaderManager->GetPixelShaderByKey(keyPrefix + L"PS");
    //        //target.Layout = ShaderManager->GetInputLayoutByKey(keyPrefix + L"VS");
    //        //target.Topology = topology;
    //    };

    //// Sphere
    //AddShaderSet(L"Sphere", "sphereVS", "spherePS", layoutPosOnly, ARRAYSIZE(layoutPosOnly),
    //    /*D3D11_PRIMITIVE_TOPOLOGY_LINELIST, Resources.Shaders.Sphere*/);

    //// Cone
    //AddShaderSet(L"Cone", "coneVS", "conePS", layoutPosOnly, ARRAYSIZE(layoutPosOnly),
    //    /*D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, Resources.Shaders.Cone*/);

    //// Icons (layout 없음)
    //AddShaderSet(L"Icon", "iconVS", "iconPS", layoutPosOnly, ARRAYSIZE(layoutPosOnly),
    //    /*D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, Resources.Shaders.Icon*/);
    ///*Resources.Shaders.Icon.Layout = nullptr;*/

    //// Arrow (기즈모 layout 재사용)
    //ShaderManager->AddVertexShaderAndInputLayout(L"ArrowVS", L"Shaders/EditorShader.hlsl", "arrowVS", layoutGizmo, ARRAYSIZE(layoutGizmo));
    //ShaderManager->AddPixelShader(L"ArrowPS", L"Shaders/EditorShader.hlsl", "arrowPS");
    //Resources.Shaders.Arrow.Vertex = ShaderManager->GetVertexShaderByKey(L"ArrowVS");
    //Resources.Shaders.Arrow.Pixel = ShaderManager->GetPixelShaderByKey(L"ArrowPS");
    //Resources.Shaders.Arrow.Layout = ShaderManager->GetInputLayoutByKey(L"ArrowVS");
    //Resources.Shaders.Arrow.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

//void FEditorRenderPass::PrepareShader(const FShaderResource& ShaderResource) const
//{
//    Graphics->DeviceContext->VSSetShader(ShaderResource.Vertex, nullptr, 0);
//    Graphics->DeviceContext->PSSetShader(ShaderResource.Pixel, nullptr, 0);
//    Graphics->DeviceContext->IASetInputLayout(ShaderResource.Layout);
//    Graphics->DeviceContext->IASetPrimitiveTopology(ShaderResource.Topology);
//}

void FEditorRenderPass::ReleaseShaders()
{


}

void FEditorRenderPass::CreateBuffers()
{
    ////////////////////////////////////
    // Sphere 버퍼 생성
    TArray<FVector> SphereFrameVertices =
    {
        {1.0, 0.0, 0},
        {0.9795299412524945, 0.20129852008866006, 0},
        {0.9189578116202306, 0.39435585511331855, 0},
        {0.8207634412072763, 0.5712682150947923, 0},
        {0.6889669190756866, 0.72479278722912, 0},
        {0.5289640103269624, 0.8486442574947509, 0},
        {0.3473052528448203, 0.9377521321470804, 0},
        {0.1514277775045767, 0.9884683243281114, 0},
        {-0.05064916883871264, 0.9987165071710528, 0},
        {-0.2506525322587204, 0.9680771188662043, 0},
        {-0.4403941515576344, 0.8978045395707416, 0},
        {-0.6121059825476629, 0.7907757369376985, 0},
        {-0.758758122692791, 0.6513724827222223, 0},
        {-0.8743466161445821, 0.48530196253108104, 0},
        {-0.9541392564000488, 0.29936312297335804, 0},
        {-0.9948693233918952, 0.10116832198743228, 0},
        {-0.9948693233918952, -0.10116832198743204, 0},
        {-0.9541392564000489, -0.29936312297335776, 0},
        {-0.8743466161445822, -0.4853019625310808, 0},
        {-0.7587581226927911, -0.651372482722222, 0},
        {-0.6121059825476627, -0.7907757369376986, 0},
        {-0.44039415155763423, -0.8978045395707417, 0},
        {-0.2506525322587205, -0.9680771188662043, 0},
        {-0.05064916883871266, -0.9987165071710528, 0},
        {0.15142777750457667, -0.9884683243281114, 0},
        {0.3473052528448203, -0.9377521321470804, 0},
        {0.5289640103269624, -0.8486442574947509, 0},
        {0.6889669190756865, -0.72479278722912, 0},
        {0.8207634412072763, -0.5712682150947924, 0},
        {0.9189578116202306, -0.3943558551133187, 0},
        {0.9795299412524945, -0.20129852008866028, 0},
        {1, 0, 0},
        {1.0, 0, 0.0},
        {0.9795299412524945, 0, 0.20129852008866006},
        {0.9189578116202306, 0, 0.39435585511331855},
        {0.8207634412072763, 0, 0.5712682150947923},
        {0.6889669190756866, 0, 0.72479278722912},
        {0.5289640103269624, 0, 0.8486442574947509},
        {0.3473052528448203, 0, 0.9377521321470804},
        {0.1514277775045767, 0, 0.9884683243281114},
        {-0.05064916883871264, 0, 0.9987165071710528},
        {-0.2506525322587204, 0, 0.9680771188662043},
        {-0.4403941515576344, 0, 0.8978045395707416},
        {-0.6121059825476629, 0, 0.7907757369376985},
        {-0.758758122692791, 0, 0.6513724827222223},
        {-0.8743466161445821, 0, 0.48530196253108104},
        {-0.9541392564000488, 0, 0.29936312297335804},
        {-0.9948693233918952, 0, 0.10116832198743228},
        {-0.9948693233918952, 0, -0.10116832198743204},
        {-0.9541392564000489, 0, -0.29936312297335776},
        {-0.8743466161445822, 0, -0.4853019625310808},
        {-0.7587581226927911, 0, -0.651372482722222},
        {-0.6121059825476627, 0, -0.7907757369376986},
        {-0.44039415155763423, 0, -0.8978045395707417},
        {-0.2506525322587205, 0, -0.9680771188662043},
        {-0.05064916883871266, 0, -0.9987165071710528},
        {0.15142777750457667, 0, -0.9884683243281114},
        {0.3473052528448203, 0, -0.9377521321470804},
        {0.5289640103269624, 0, -0.8486442574947509},
        {0.6889669190756865, 0, -0.72479278722912},
        {0.8207634412072763, 0, -0.5712682150947924},
        {0.9189578116202306, 0, -0.3943558551133187},
        {0.9795299412524945, 0, -0.20129852008866028},
        {1, 0, 0},
        {0, 1.0, 0.0},
        {0, 0.9795299412524945, 0.20129852008866006},
        {0, 0.9189578116202306, 0.39435585511331855},
        {0, 0.8207634412072763, 0.5712682150947923},
        {0, 0.6889669190756866, 0.72479278722912},
        {0, 0.5289640103269624, 0.8486442574947509},
        {0, 0.3473052528448203, 0.9377521321470804},
        {0, 0.1514277775045767, 0.9884683243281114},
        {0, -0.05064916883871264, 0.9987165071710528},
        {0, -0.2506525322587204, 0.9680771188662043},
        {0, -0.4403941515576344, 0.8978045395707416},
        {0, -0.6121059825476629, 0.7907757369376985},
        {0, -0.758758122692791, 0.6513724827222223},
        {0, -0.8743466161445821, 0.48530196253108104},
        {0, -0.9541392564000488, 0.29936312297335804},
        {0, -0.9948693233918952, 0.10116832198743228},
        {0, -0.9948693233918952, -0.10116832198743204},
        {0, -0.9541392564000489, -0.29936312297335776},
        {0, -0.8743466161445822, -0.4853019625310808},
        {0, -0.7587581226927911, -0.651372482722222},
        {0, -0.6121059825476627, -0.7907757369376986},
        {0, -0.44039415155763423, -0.8978045395707417},
        {0, -0.2506525322587205, -0.9680771188662043},
        {0, -0.05064916883871266, -0.9987165071710528},
        {0, 0.15142777750457667, -0.9884683243281114},
        {0, 0.3473052528448203, -0.9377521321470804},
        {0, 0.5289640103269624, -0.8486442574947509},
        {0, 0.6889669190756865, -0.72479278722912},
        {0, 0.8207634412072763, -0.5712682150947924},
        {0, 0.9189578116202306, -0.3943558551133187},
        {0, 0.9795299412524945, -0.20129852008866028},
        {0, 1, 0}
    };

    TArray<uint32> SphereFrameIndices =
    {
        0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
        11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20,
        21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30,
        31, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40, 40,
        41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 50, 50,
        51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57, 58, 58, 59, 59, 60, 60,
        61, 61, 62, 62, 63, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 69, 70, 70,
        71, 71, 72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 79, 80, 80,
        81, 81, 82, 82, 83, 83, 84, 84, 85, 85, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90,
        91, 91, 92, 92, 93, 93, 94, 94, 95
    };

    // 버텍스 버퍼 생성
    BufferManager->CreateVertexBuffer(SphereKey, SphereFrameVertices);
    BufferManager->CreateIndexBuffer(SphereKey, SphereFrameIndices);
    

    TArray<FVector> ConeVertices;
    ConeVertices.Add({ 0.0f, 0.0f, 0.0f }); // Apex
    for (int i = 0; i < NumConeSegments; i++)
    {
        // hlsl 내부에서 계산
        //float angle = 2.0f * 3.1415926535897932f * i / (float)NuzmSegments;
        //float x = cos(angle);
        //float y = sin(angle);
        float x = 0;
        float y = 0;
        ConeVertices.Add({ x, y, 1.0f }); // Bottom
    }
    TArray<uint32> ConeIndices;
    uint32 vertexOffset0 = 1;
    // apex -> 밑면으로 가는 line
    for (int i = 0; i < NumConeSegments; i++)
    {
        ConeIndices.Add(0);
        ConeIndices.Add(vertexOffset0 + i);
    }
    // 밑변
    for (int i = 0; i < NumConeSegments; i++)
    {
        ConeIndices.Add(vertexOffset0 + i);
        ConeIndices.Add(vertexOffset0 + (i + 1) % NumConeSegments);
    }

    // cone을 덮는 sphere
    // xz plane
    //float deltaAngle = 2.0f * 3.1415926535897932f / (float)NumConeSegments;
    //float offsetAngle = deltaAngle * NumConeSegments / 8; // 45도 부터 시작
    for (int i = 0; i < NumConeSegments + 1; i++)
    {
        //float angle = 2.0f * 3.1415926535897932f * i / (float)NumConeSegments + offsetAngle;
        //float x = cos(angle) * sqrt(2.f);
        //float z = sin(angle) * sqrt(2.f);
        float x = 0;
        float z = 0;
        ConeVertices.Add({ x, 0, z });
    }
    uint32 vertexOffset1 = NumConeSegments + vertexOffset0;
    for (int i = 0; i < NumConeSegments; i++)
    {
        ConeIndices.Add(vertexOffset1 + i);
        ConeIndices.Add(vertexOffset1 + (i + 1));
    }
    // yz plane
    for (int i = 0; i < NumConeSegments + 1; i++)
    {
        //float angle = 2.0f * 3.1415926535897932f * i / (float)NumConeSegments + offsetAngle;
        //float y = cos(angle) * sqrt(2.f);
        //float z = sin(angle) * sqrt(2.f);
        float y = 0;
        float z = 0;
        ConeVertices.Add({ 0, y, z });
    }
    uint32 vertexOffset2 = NumConeSegments + 1 + vertexOffset1;
    for (int i = 0; i < NumConeSegments; i++)
    {
        ConeIndices.Add(vertexOffset2 + i);
        ConeIndices.Add(vertexOffset2 + (i + 1));
    }
    BufferManager->CreateIndexBuffer(ConeKey, ConeIndices);

    ////////////////////////////////////
    // Box 버퍼 생성
    TArray<FVector> CubeFrameVertices;
    CubeFrameVertices.Add({ -1.f, -1.f, -1.f }); // 0
    CubeFrameVertices.Add({ -1.f, 1.f, -1.f }); // 1
    CubeFrameVertices.Add({ 1.f, -1.f, -1.f }); // 2
    CubeFrameVertices.Add({ 1.f, 1.f, -1.f }); // 3
    CubeFrameVertices.Add({ -1.f, -1.f, 1.f }); // 4
    CubeFrameVertices.Add({ 1.f, -1.f, 1.f }); // 5
    CubeFrameVertices.Add({ -1.f, 1.f, 1.f }); // 6
    CubeFrameVertices.Add({ 1.f, 1.f, 1.f }); // 7

    TArray<uint32> CubeFrameIndices = {
        // Bottom face
        0, 1, 1, 3, 3, 2, 2, 0,
        // Top face
        4, 6, 6, 7, 7, 5, 5, 4,
        // Side faces
        0, 4, 1, 6, 2, 5, 3, 7
    };

    BufferManager->CreateVertexBuffer(AABBKey, CubeFrameVertices);
    BufferManager->CreateIndexBuffer(AABBKey, CubeFrameIndices);
}
void FEditorRenderPass::CreateConstantBuffers()
{
    BufferManager->CreateConstantBuffer<FConstantBufferDebugSphere>(SphereKey, ConstantBufferSizeSphere);
    BufferManager->CreateConstantBuffer<FConstantBufferDebugCone>(ConeKey, ConstantBufferSizeCone);
    BufferManager->CreateConstantBuffer<FConstantBufferDebugAABB>(AABBKey, ConstantBufferSizeAABB);
    BufferManager->CreateConstantBuffer<FConstantBufferDebugGrid>(GridKey);
    BufferManager->CreateConstantBuffer<FConstantBufferDebugArrow>(ArrowKey);
    BufferManager->CreateConstantBuffer<FConstantBufferDebugIcon>(IconKey, ConstantBufferSizeIcon);
    //CreateCB(sizeof(FConstantBufferCamera), &Resources.ConstantBuffers.AABBKeyCamera00);
    //CreateCB(sizeof(FConstantBufferDebugAABB) * ConstantBufferSizeAABB, &Resources.ConstantBuffers.AABB11);
    //CreateCB(sizeof(FConstantBufferDebugSphere) * ConstantBufferSizeSphere, &Resources.ConstantBuffers.Sphere11);
    //CreateCB(sizeof(FConstantBufferDebugCone) * ConstantBufferSizeCone, &Resources.ConstantBuffers.Cone11);
    //CreateCB(sizeof(FConstantBufferDebugGrid), &Resources.ConstantBuffers.Grid11);
    //CreateCB(sizeof(FConstantBufferDebugIcon), &Resources.ConstantBuffers.Icon11);
    //CreateCB(sizeof(FConstantBufferDebugArrow), &Resources.ConstantBuffers.Arrow11);
}

// 이전 state와 상관없이 업데이트
void FEditorRenderPass::PrepareRendertarget(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    const EResourceType ResourceType = EResourceType::ERT_Editor;

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, ViewportResource->GetDepthStencilView());
}

void FEditorRenderPass::PrepareRender()
{
    ClearRenderArr();
    // gizmo 제외하고 넣기

    if (GEngine->ActiveWorld->WorldType != EWorldType::Editor)
    {
        return;
    }

    UEditorEngine* EditorEngine = Cast< UEditorEngine>(GEngine);
    if (!EditorEngine)
    {
        return;
    }

    for (const auto iter : TObjectRange<UStaticMeshComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            Resources.Components.StaticMesh.Add(iter);
        }
    }

    for (const auto iter : TObjectRange<UDirectionalLightComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            Resources.Components.DirLight.Add(iter);
        }
    }

    for (const auto iter : TObjectRange<UPointLightComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            Resources.Components.PointLight.Add(iter);
        }
    }


    for (const auto iter : TObjectRange<USpotLightComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            Resources.Components.SpotLight.Add(iter);
        }
    }

    for (const auto iter : TObjectRange<UHeightFogComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            Resources.Components.Fog.Add(iter);
        }
    }

}

void FEditorRenderPass::ClearRenderArr()
{
    Resources.Components.StaticMesh.Empty();
    Resources.Components.DirLight.Empty();
    Resources.Components.PointLight.Empty();
    Resources.Components.SpotLight.Empty();
    Resources.Components.Fog.Empty();
}

void FEditorRenderPass::ReloadShader()
{
}

void FEditorRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    static bool isLoaded = false;
    if (!isLoaded)
    {
        LazyLoad();
        isLoaded = true;
    }

    PrepareRendertarget(Viewport);
    RenderGrid(Viewport);
    RenderAxis();
    if (Viewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
    {
        RenderAABBInstanced();
    }

    RenderPointlightInstanced();
    RenderSpotlightInstanced();
    RenderArrows();

    //ID3D11DepthStencilState* DepthStateEnable = Graphics->DepthStencilStateTestWriteEnable;
    //Graphics->DeviceContext->OMSetDepthStencilState(DepthStateEnable, 0);

    if (Viewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
    {
        RenderIcons(Viewport);
    }
    //ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStencilStateTestWriteDisable;
    //Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
}

void FEditorRenderPass::RenderAxis()
{
    // vertex index buffer 없음
    ShaderManager->SetVertexShader(AxisKeyW);
    ShaderManager->SetPixelShader(AxisKeyW);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    Graphics->DeviceContext->Draw(6, 0);
}
void FEditorRenderPass::RenderAABBInstanced()
{

    ShaderManager->SetVertexShaderAndInputLayout(AABBKeyW);
    ShaderManager->SetPixelShader(AABBKeyW);
    BufferManager->SetVertexBuffer(AABBKey);    
    BufferManager->SetIndexBuffer(AABBKey);
    BufferManager->BindConstantBuffer(AABBKey, 11, EShaderStage::Vertex);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    uint32 NumIndices = BufferManager->GetIndexBuffer(AABBKey).NumIndices;

    // 위치랑 bounding box 크기 정보 가져오기
    TArray<FConstantBufferDebugAABB> BufferAll;
    for (UStaticMeshComponent* SMComp : Resources.Components.StaticMesh)
    {
        // 매 프레임마다 하고있는데, scenecomponent에서 변경이 발생하면 갱신하도록 변경필요
        if (SMComp->GetStaticMesh() == nullptr)
            return;
        FVector min = FVector(FLT_MAX, FLT_MAX, FLT_MAX);
        FVector max = FVector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        for (const FStaticMeshVertex& Vertex : SMComp->GetStaticMesh()->GetRenderData()->Vertices)
        {
            FVector VertexWorld = FVector(Vertex.X, Vertex.Y, Vertex.Z);
            VertexWorld = SMComp->GetWorldMatrix().TransformPosition(VertexWorld);
            min.X = std::min(min.X, VertexWorld.X);
            min.Y = std::min(min.Y, VertexWorld.Y);
            min.Z = std::min(min.Z, VertexWorld.Z);
            max.X = std::max(max.X, VertexWorld.X);
            max.Y = std::max(max.Y, VertexWorld.Y);
            max.Z = std::max(max.Z, VertexWorld.Z);
        }
        FConstantBufferDebugAABB b;
        b.Position = (min + max) / 2.f;
        b.Extent = (max - min) / 2.f;
        BufferAll.Add(b);
    }

    int BufferIndex = 0;
    for (int i = 0; i < (1 + BufferAll.Num() / ConstantBufferSizeAABB) * ConstantBufferSizeAABB; ++i)
    {
        TArray<FConstantBufferDebugAABB> SubBuffer;
        for (int j = 0; j < ConstantBufferSizeAABB; ++j)
        {
            if (BufferIndex < BufferAll.Num())
            {
                SubBuffer.Add(BufferAll[BufferIndex]);
                ++BufferIndex;
            }
            else
            {
                break;
            }
        }

        if (SubBuffer.Num() > 0)
        {
            BufferManager->UpdateConstantBufferArray(AABBKey, SubBuffer);
            Graphics->DeviceContext->DrawIndexedInstanced(NumIndices, SubBuffer.Num(), 0, 0, 0);
        }
    }
}


void FEditorRenderPass::RenderPointlightInstanced()
{
    ShaderManager->SetVertexShaderAndInputLayout(SphereKeyW);
    ShaderManager->SetPixelShader(SphereKeyW);
    BufferManager->SetVertexBuffer(SphereKey);
    BufferManager->SetIndexBuffer(SphereKey);
    BufferManager->BindConstantBuffer(SphereKey, 11, EShaderStage::Vertex);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    TArray<FConstantBufferDebugSphere> BufferAll;
    // 하나만 뜨도록 설정함
    if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
    {
        for (UPointLightComponent* PointLightComp : Resources.Components.PointLight)
        {
            if (PointLightComp->GetOwner() == EditorEngine->GetSelectedActor())
            {
                FConstantBufferDebugSphere b;
                b.Position = PointLightComp->GetWorldLocation();
                b.Radius = PointLightComp->GetRadius();
                BufferAll.Add(b);
                break;
            }
        }
    }

    int BufferIndex = 0;
    uint32 NumIndices = BufferManager->GetIndexBuffer(SphereKey).NumIndices;
    for (int i = 0; i < (1 + BufferAll.Num() / ConstantBufferSizeSphere) * ConstantBufferSizeSphere; ++i)
    {
        TArray<FConstantBufferDebugSphere> SubBuffer;
        for (int j = 0; j < ConstantBufferSizeAABB; ++j)
        {
            if (BufferIndex < BufferAll.Num())
            {
                SubBuffer.Add(BufferAll[BufferIndex]);
                ++BufferIndex;
            }
            else
            {
                break;
            }
        }

        if (SubBuffer.Num() > 0)
        {
            BufferManager->UpdateConstantBufferArray(SphereKey, SubBuffer);
            Graphics->DeviceContext->DrawIndexedInstanced(NumIndices, SubBuffer.Num(), 0, 0, 0);
        }
    }
}

void FEditorRenderPass::RenderSpotlightInstanced()
{
    Graphics->DeviceContext->IASetInputLayout(nullptr);
    ShaderManager->SetVertexShader(ConeKeyW);
    ShaderManager->SetPixelShader(ConeKeyW);
    BufferManager->SetIndexBuffer(ConeKey);
    BufferManager->BindConstantBuffer(ConeKey, 11, EShaderStage::Vertex);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    constexpr FLinearColor OuterColor = FLinearColor(149.f / 255.f, 198.f / 255.f, 255.f / 255.f, 255.f / 255.f);
    constexpr FLinearColor InnerColor = FLinearColor(0.777f, 1.0f, 1.0f, 1.0f);

    // 위치랑 bounding box 크기 정보 가져오기
    TArray<FConstantBufferDebugCone> BufferAll;

    // 하나만 뜨도록 설정함
    if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
    {
        for (USpotLightComponent* SpotComp : Resources.Components.SpotLight)
        {
            if (SpotComp->GetOwner() == EditorEngine->GetSelectedActor())
            {
                FConstantBufferDebugCone b;
                b.ApexPosiiton = SpotComp->GetWorldLocation();
                b.Radius = SpotComp->GetRadius();
                b.Direction = SpotComp->GetDirection();
                b.Angle = SpotComp->GetInnerRad();
                b.Color = InnerColor;
                BufferAll.Add(b);

                b.Angle = SpotComp->GetOuterRad();
                b.Color = OuterColor;
                BufferAll.Add(b);
                break;
            }
        }
    }

    int BufferIndex = 0;
    uint32 NumIndices = BufferManager->GetIndexBuffer(ConeKey).NumIndices;

    for (int i = 0; i < (1 + BufferAll.Num() / ConstantBufferSizeCone) * ConstantBufferSizeCone; ++i)
    {
        TArray<FConstantBufferDebugCone> SubBuffer;
        for (int j = 0; j < ConstantBufferSizeCone; ++j)
        {
            if (BufferIndex < BufferAll.Num())
            {
                SubBuffer.Add(BufferAll[BufferIndex]);
                ++BufferIndex;
            }
            else
            {
                break;
            }
        }

        if (SubBuffer.Num() > 0)
        {
            BufferManager->UpdateConstantBufferArray(ConeKey, SubBuffer);

            Graphics->DeviceContext->DrawIndexedInstanced(NumIndices, SubBuffer.Num(),0,0,0);
        }
    }
}

void FEditorRenderPass::RenderGrid(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->IASetInputLayout(nullptr);
    ShaderManager->SetVertexShader(GridKeyW);
    ShaderManager->SetPixelShader(GridKeyW);
    BufferManager->BindConstantBuffer(GridKey, 11, EShaderStage::Vertex);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    float Spacing = Viewport->GetGridSize() * 10;
    FVector GridOrigin = Viewport->GetCameraLocation();
    GridOrigin.X = std::floorf((GridOrigin.X - KINDA_SMALL_NUMBER) / Spacing) * Spacing;
    GridOrigin.Y = std::floorf((GridOrigin.Y - KINDA_SMALL_NUMBER) / Spacing) * Spacing;
    GridOrigin.Z = 0;

    FConstantBufferDebugGrid b;

    b.GridOrigin = GridOrigin;
    b.GridCount = 32;
    b.GridSpacing = Spacing;
    b.Color = 0.5f;
    b.Alpha = 0.5f;
    BufferManager->UpdateConstantBuffer(GridKey, b);
    Graphics->DeviceContext->DrawInstanced(2, b.GridCount, 0, 0); // 내부에서 버텍스 사용중

    b.GridCount *= 10;
    b.GridSpacing /= 10;
    b.Color = 0.3f;
    b.Alpha = 0.5f;
    BufferManager->UpdateConstantBuffer(GridKey, b);
    Graphics->DeviceContext->DrawInstanced(2, b.GridCount, 0, 0); // 내부에서 버텍스 사용중

    b.GridCount *= 10;
    b.GridSpacing /= 10;
    b.Color = 0.1f;
    b.Alpha = 0.5f;
    BufferManager->UpdateConstantBuffer(GridKey, b);
    Graphics->DeviceContext->DrawInstanced(2, b.GridCount, 0, 0); // 내부에서 버텍스 사용중
}

//// 꼼수로 이미 로드된 리소스를 사용
//// GUObjectArray에 안올라가게 우회
void FEditorRenderPass::LazyLoad()
{
    // Resourcemanager에서 로드된 texture의 포인터를 가져옴
    // FResourceMgr::Initialize에 추가되어야함
    Resources.IconTextures[IconType::DirectionalLight] = FEngineLoop::ResourceManager.GetTexture(L"Assets/Editor/Icon/DirectionalLight_64x.png");
    Resources.IconTextures[IconType::PointLight] = FEngineLoop::ResourceManager.GetTexture(L"Assets/Editor/Icon/PointLight_64x.png");
    Resources.IconTextures[IconType::SpotLight] = FEngineLoop::ResourceManager.GetTexture(L"Assets/Editor/Icon/SpotLight_64x.png");
    Resources.IconTextures[IconType::AmbientLight] = FEngineLoop::ResourceManager.GetTexture(L"Assets/Editor/Icon/AmbientLight_64x.png");
    Resources.IconTextures[IconType::ExponentialFog] = FEngineLoop::ResourceManager.GetTexture(L"Assets/Editor/Icon/ExponentialHeightFog_64.png");
    Resources.IconTextures[IconType::AtmosphericFog] = FEngineLoop::ResourceManager.GetTexture(L"Assets/Editor/Icon/AtmosphericFog_64.png");

    // Gizmo arrow 로드
    UStaticMesh* Mesh = FManagerOBJ::GetStaticMesh(L"Assets/GizmoTranslationZ.obj");
    BufferManager->CreateVertexBuffer(ArrowKey, Mesh->GetRenderData()->Vertices);
    BufferManager->CreateIndexBuffer(ArrowKey, Mesh->GetRenderData()->Indices);
}

void FEditorRenderPass::RenderIcons(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->IASetInputLayout(nullptr);
    ShaderManager->SetVertexShader(IconKeyW);
    ShaderManager->SetPixelShader(IconKeyW);
    BufferManager->BindConstantBuffer(IconKey, 11, EShaderStage::Vertex);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const float IconScale = 1.f;
    AActor* PickedActor = nullptr;
    if (UEditorEngine* Engine = Cast<UEditorEngine>(GEngine))
    {
        if (AEditorPlayer* player = Engine->GetEditorPlayer())
        {
            PickedActor = Engine->GetSelectedActor();
        }
    }

    Graphics->DeviceContext->PSSetShaderResources(0, 1, &Resources.IconTextures[IconType::DirectionalLight]->TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &Resources.IconTextures[IconType::DirectionalLight]->SamplerState);
    for (UDirectionalLightComponent* DirLightComp : Resources.Components.DirLight)
    {
        FConstantBufferDebugIcon b;
        b.Position = DirLightComp->GetWorldLocation();
        b.Scale = IconScale;
        if (DirLightComp->GetOwner() == PickedActor)
        {
            b.Color = FLinearColor(0.5, 0.5, 0, 1);
        }
        else
        {
            b.Color = DirLightComp->GetLightColor();
        }
        BufferManager->UpdateConstantBuffer(IconKey, b);
        Graphics->DeviceContext->Draw(6, 0); // 내부에서 버텍스 사용중
    }

    Graphics->DeviceContext->PSSetShaderResources(0, 1, &Resources.IconTextures[IconType::PointLight]->TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &Resources.IconTextures[IconType::PointLight]->SamplerState);
    TArray<FConstantBufferDebugIcon> PointLightBuffers;
    for (UPointLightComponent* PointLightComp : Resources.Components.PointLight)
    {
        FConstantBufferDebugIcon b;
        b.Position = PointLightComp->GetWorldLocation();
        b.Scale = IconScale;
        if (PointLightComp->GetOwner() == PickedActor)
        {
            b.Color = FLinearColor(0.5, 0.5, 0, 1);
        }
        else
        {
            b.Color = PointLightComp->GetLightColor();
        }
        PointLightBuffers.Add(b);
    }

    int BufferIndex = 0;
    for (int i = 0; i < (1 + PointLightBuffers.Num() / ConstantBufferSizeIcon) * ConstantBufferSizeIcon; ++i)
    {
        TArray<FConstantBufferDebugIcon> SubBuffer;
        for (int j = 0; j < ConstantBufferSizeIcon; ++j)
        {
            if (BufferIndex < PointLightBuffers.Num())
            {
                SubBuffer.Add(PointLightBuffers[BufferIndex]);
                ++BufferIndex;
            }
            else
            {
                break;
            }
        }

        if (SubBuffer.Num() > 0)
        {
            BufferManager->UpdateConstantBufferArray(IconKey, SubBuffer);
            Graphics->DeviceContext->DrawInstanced(6, SubBuffer.Num(), 0, 0);
        }
    }

    Graphics->DeviceContext->PSSetShaderResources(0, 1, &Resources.IconTextures[IconType::SpotLight]->TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &Resources.IconTextures[IconType::SpotLight]->SamplerState);
    for (USpotLightComponent* SpotLightComp : Resources.Components.SpotLight)
    {
        FConstantBufferDebugIcon b;
        b.Position = SpotLightComp->GetWorldLocation();
        b.Scale = IconScale;
        if (SpotLightComp->GetOwner() == PickedActor)
        {
            b.Color = FLinearColor(0.5, 0.5, 0, 1);
        }
        else
        {
            b.Color = SpotLightComp->GetLightColor();
        }
        BufferManager->UpdateConstantBuffer(IconKey, b);
        Graphics->DeviceContext->Draw(6, 0); // 내부에서 버텍스 사용중
    }

    Graphics->DeviceContext->PSSetShaderResources(0, 1, &Resources.IconTextures[IconType::ExponentialFog]->TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &Resources.IconTextures[IconType::ExponentialFog]->SamplerState);
    for (UHeightFogComponent* FogComp : Resources.Components.Fog)
    {
        FConstantBufferDebugIcon b;
        b.Position = FogComp->GetWorldLocation();
        b.Scale = IconScale;
        b.Color = FogComp->GetFogColor();

        BufferManager->UpdateConstantBuffer(IconKey, b);
        Graphics->DeviceContext->Draw(6, 0); // 내부에서 버텍스 사용중
    }
}

void FEditorRenderPass::RenderArrows()
{
    ShaderManager->SetVertexShaderAndInputLayout(ArrowKeyW);
    ShaderManager->SetPixelShader(ArrowKeyW);
    BufferManager->SetVertexBuffer(ArrowKey);
    BufferManager->SetIndexBuffer(ArrowKey);
    BufferManager->BindConstantBuffer(ArrowKey, 11, EShaderStage::Vertex);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // XYZ한번. Z는 중복으로 적용
    const float ArrowScale = 1.5;
    uint32 NumIndices = BufferManager->GetIndexBuffer(ArrowKey).NumIndices;

    for (UDirectionalLightComponent* DLightComp : Resources.Components.DirLight)
    {
        FConstantBufferDebugArrow buf;
        buf.Position = DLightComp->GetWorldLocation();
        buf.ArrowScaleXYZ = ArrowScale;
        buf.Direction = DLightComp->GetDirection();
        buf.ArrowScaleZ = ArrowScale;
        buf.Color = DLightComp->GetLightColor();
        BufferManager->UpdateConstantBuffer(ArrowKey, buf);
        Graphics->DeviceContext->DrawIndexed(NumIndices, 0, 0);

    }
    for (USpotLightComponent* SLightComp : Resources.Components.SpotLight)
    {
        FConstantBufferDebugArrow buf;
        buf.Position = SLightComp->GetWorldLocation();
        buf.ArrowScaleXYZ = ArrowScale;
        buf.Direction = SLightComp->GetDirection();
        buf.ArrowScaleZ = ArrowScale;
        buf.Color = SLightComp->GetLightColor();
        BufferManager->UpdateConstantBuffer(ArrowKey, buf);
        Graphics->DeviceContext->DrawIndexed(NumIndices, 0, 0);

    }

}
