#pragma once
#define _TCHAR_DEFINED
#include <d3dcommon.h>
#include <filesystem>

#include "ShaderFileMetadata.h"
#include "Container/Array.h"
#include "Container/Pair.h"
#include "Container/String.h"


enum class EShaderType : uint8
{
    VertexShader,
    PixelShader

    // 나중에 다른 셰이더도 추가되면 그때 추가
};

inline const char* ToString(EShaderType e)
{
    switch (e)
    {
    case EShaderType::VertexShader: return "VertexShader";
    case EShaderType::PixelShader: return "PixelShader";
    default: return "unknown";  // NOLINT(clang-diagnostic-covered-switch-default)
    }
}

/** EShaderType에 따른 TargetProfile을 반환합니다. */
const char* ShaderTypeToTargetProfile(EShaderType ShaderType);


/** 셰이더 컴파일 작업 정보 구조체 */
struct FShaderCompileJob
{
    std::wstring ShaderKey;

    /** .hlsl의 파일 경로 */
    std::filesystem::path FilePath;

    /** 셰이더 Defines */
    TArray<TPair<FString, FString>> Defines;

    /** EntryPoint */
    FString EntryPoint;

    /** 컴파일 할 Shader Type */
    EShaderType Type;
};

/** 셰이더 컴파일 결과 구조체 */
struct FShaderCompileResult
{
    std::wstring ShaderKey;
    ID3DBlob* CsoBlob;
    ID3DBlob* ErrorBlob;
    TArray<FFileMetadata> IncludePaths;
    EShaderType Type;
    bool bSuccess;
};
