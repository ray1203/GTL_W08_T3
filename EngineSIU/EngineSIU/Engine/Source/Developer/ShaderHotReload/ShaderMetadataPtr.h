#pragma once
#define _TCHAR_DEFINED
#include <d3d11.h>
#include <filesystem>
#include <type_traits>

#include "ShaderFileMetadata.h"
#include "Container/Array.h"
#include "HAL/PlatformType.h"


template <typename T>
concept ShaderPtrConcept =
    std::is_same_v<T, ID3D11ComputeShader>
    || std::is_same_v<T, ID3D11VertexShader>
    || std::is_same_v<T, ID3D11PixelShader>
    || std::is_same_v<T, ID3D11GeometryShader>
    || std::is_same_v<T, ID3D11HullShader>
    || std::is_same_v<T, ID3D11DomainShader>;

/**
 * Shader Hot Reload를 위한, 파일 정보를 담고 있는 구조체입니다.
 */
template <ShaderPtrConcept ShaderType>
class TShaderMetadataPtr
{
public:
    TShaderMetadataPtr()
        : ShaderPtr(nullptr)
    {
    }

    explicit TShaderMetadataPtr(nullptr_t)
        : ShaderPtr(nullptr)
    {
    }

    explicit TShaderMetadataPtr(ShaderType* RawShaderPtr)
        : ShaderPtr(RawShaderPtr)
    {
    }

    TShaderMetadataPtr& operator=(ShaderType* RawShaderPtr)
    {
        ShaderPtr = RawShaderPtr;
        return *this;
    }

    TShaderMetadataPtr& operator=(nullptr_t)
    {
        ShaderPtr = nullptr;
        return *this;
    }

    FORCEINLINE ShaderType* Get() const { return ShaderPtr; }

    FORCEINLINE ShaderType* operator->() { return Get(); }
    FORCEINLINE ShaderType& operator*() { return *Get(); }
    FORCEINLINE operator ShaderType*() const { return Get(); }
    FORCEINLINE operator bool() const { return Get(); }

public:
    void SetMetadata(std::unique_ptr<FShaderFileMetadata> InMetadata)
    {
        Metadata = std::move(InMetadata);
    }

    [[nodiscard]] FShaderFileMetadata& GetMetadata() const
    {
        return *Metadata;
    }

private:
    ShaderType* ShaderPtr;
    std::unique_ptr<FShaderFileMetadata> Metadata;
};
