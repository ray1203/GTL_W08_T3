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

    //void LazyLoad();
    
    void PrepareRendertarget(const std::shared_ptr<FEditorViewportClient>& Viewport);


    // constant buffer용 
    FString CameraKey = FString("DebugCamera");
    // Gizmo 관련 함수
    //void RenderGizmos(const UWorld* World);
    //void PrepareShaderGizmo();
    //void PrepareConstantbufferGizmo();

    // Axis
    const FWString AxisKeyW = L"DebugArrow";
    const FString AxisKey = FString(AxisKeyW);
    void RenderAxis();

    // AABB
    //void RenderAABBInstanced(const UWorld* World);
    //void PrepareConstantbufferAABB();
    //void UdpateConstantbufferAABBInstanced(TArray<FConstantBufferDebugAABB> Buffer);

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
    // void RenderGrid(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    // void PrepareConstantbufferGrid();
    // void UpdateConstantbufferGrid(FConstantBufferDebugGrid Buffer);

    //// Icon
    //void RenderIcons(const UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    //void PrepareConstantbufferIcon();
    //void UdpateConstantbufferIcon(const FConstantBufferDebugIcon& Buffer);
    //void UpdateTextureIcon(IconType type);

    //// Arrow
    //void RenderArrows();
    //void PrepareConstantbufferArrow();
    //void UdpateConstantbufferArrow(const FConstantBufferDebugArrow& Buffer);

    const UINT32 ConstantBufferSizeAABB = 8;
    const UINT32 ConstantBufferSizeSphere = 8;
    const UINT32 ConstantBufferSizeCone = 16; // 최대

};

