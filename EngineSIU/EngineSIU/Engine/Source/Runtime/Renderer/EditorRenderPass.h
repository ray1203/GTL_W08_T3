#pragma once
#define _TCHAR_DEFINED
#include <d3d11.h>
#include "Math/Matrix.h"
#include "Container/Set.h"
#include "RenderResources.h"

class FDXDBufferManager;
class FGraphicsDevice;
class UWorld;
class FEditorViewportClient;
class FDXDShaderManager;

class FEditorRenderPass
{
public:
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager);
    void Render(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void Release();

    void PrepareRender();

    void ClearRenderArr();

    void ReloadShader();

private:
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;
    FRenderResourcesDebug Resources;

    void CreateShaders();
    //void PrepareShader(const FShaderResource& ShaderResource) const;
    void ReleaseShaders();

    void CreateBuffers();
    void CreateConstantBuffers();

    void LazyLoad();
    
    void PrepareRendertarget(const std::shared_ptr<FEditorViewportClient>& Viewport);

    // Gizmo 관련 함수
    //void RenderGizmos(const UWorld* World);
    //void PrepareShaderGizmo();
    //void PrepareConstantbufferGizmo();

    // Axis
    const FWString AxisKeyW = L"DebugAxis";
    const FString AxisKey = FString(AxisKeyW);
    void RenderAxis();

    // AABB
    struct FConstantBufferDebugAABB
    {
        FVector Position;
        float Padding1;

        FVector Extent;
        float Padding2;
    };
    const FWString AABBKeyW = L"DebugAABB";
    const FString AABBKey = FString(AABBKeyW);
    void RenderAABBInstanced();
    
    // Sphere
    struct alignas(16) FConstantBufferDebugSphere
    {
        FVector Position;
        float Radius;
    };
    const FWString SphereKeyW = L"DebugSphere";
    const FString SphereKey = FString(SphereKeyW);
    void RenderPointlightInstanced();
     
    //// Cone
    struct alignas(16) FConstantBufferDebugCone
    {
        alignas(16) FVector ApexPosiiton;
        float Radius;
        FVector Direction;
        float Angle;
        FLinearColor Color;
    };
    const uint32 NumConeSegments = 16;
    const FWString ConeKeyW = L"DebugCone";
    const FString ConeKey = FString(ConeKeyW); // vertex / index 버퍼 없음
    void RenderSpotlightInstanced();

    // Grid
    struct alignas(16) FConstantBufferDebugGrid
    {
        FVector GridOrigin; // Grid의 중심
        float GridSpacing;
        int GridCount; // 총 grid 라인 수
        float Color;
        float Alpha; // 적용 안됨
        float padding;
    };
    const FWString GridKeyW = L"DebugGrid";
    const FString GridKey = FString(GridKeyW);
    void RenderGrid(const std::shared_ptr<FEditorViewportClient>& Viewport);

    //// Icon
    struct alignas(16) FConstantBufferDebugIcon
    {
        alignas(16) FVector Position;
        float Scale;
        FLinearColor Color;
    };
    const FWString IconKeyW = L"DebugIcon";
    const FString IconKey = FString(IconKeyW);
    void RenderIcons(const std::shared_ptr<FEditorViewportClient>& Viewport);
    //void PrepareConstantbufferIcon();
    //void UdpateConstantbufferIcon(const FConstantBufferDebugIcon& Buffer);
    //void UpdateTextureIcon(IconType type);

    //// Arrow
    struct FConstantBufferDebugArrow
    {
        alignas(16) FVector Position;
        float ArrowScaleXYZ;
        alignas(16) FVector Direction;
        float ArrowScaleZ;
        FLinearColor Color;
    };
    const FWString ArrowKeyW = L"DebugArrow";
    const FString ArrowKey = FString(ArrowKeyW);
    void RenderArrows();

    const UINT32 ConstantBufferSizeAABB = 8;
    const UINT32 ConstantBufferSizeSphere = 8;
    const UINT32 ConstantBufferSizeCone = 16; // 최대
    const uint32 ConstantBufferSizeIcon = 16;

};

