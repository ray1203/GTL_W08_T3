#pragma once

#include <memory>
#define _TCHAR_DEFINED
#include <d3d11.h>
#include "Core/Container/Array.h"
#include "Core/Container/Map.h"
#include "ShaderConstants.h"
#include "Engine/Classes/Engine/Texture.h"

/// <summary>
/// Shader관련 모음.
/// VS, PS, InputLayout
/// 상수 버퍼 관련은 ShaderConstants.h로
/// </summary>
//struct FShaderResource
//{
//    ID3D11VertexShader* Vertex = nullptr;
//    ID3D11PixelShader* Pixel = nullptr;
//    ID3D11InputLayout* Layout = nullptr;
//    D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//};


struct FRenderResources
{
    struct FWorldComponentContainer
    {
        TArray<class UStaticMeshComponent*> StaticMeshObjs;
        TArray<class UBillboardComponent*> BillboardObjs;
        TArray<class UTextRenderComponent*> TextObjs;
        TArray<class ULightComponentBase*> LightObjs;
    } Components;

};

//struct FDebugPrimitiveData
//{
//    ID3D11Buffer* Vertex;
//    ID3D11Buffer* Index;
//    UINT32 NumVertices;
//    UINT32 VertexStride;
//    UINT32 NumIndices;
//};

// Icon
enum class IconType
{
    None,
    DirectionalLight,
    PointLight,
    SpotLight,
    AmbientLight,
    ExponentialFog,
    AtmosphericFog,
};

struct FRenderResourcesDebug
{
    struct FWorldComponentContainer
    {
        TArray<class UStaticMeshComponent*> StaticMesh;
        TArray<class UDirectionalLightComponent*> DirLight;
        TArray<class USpotLightComponent*> SpotLight;
        TArray<class UPointLightComponent*> PointLight;
        TArray<class UHeightFogComponent*> Fog;
    } Components;

    //struct FPrimitiveResourceContainer
    //{
    //    FDebugPrimitiveData Box;
    //    FDebugPrimitiveData Sphere;
    //    FDebugPrimitiveData Cone;
    //    FDebugPrimitiveData Arrow;
    //} Primitives;

    TMap<IconType, std::shared_ptr<FTexture>> IconTextures;
};
