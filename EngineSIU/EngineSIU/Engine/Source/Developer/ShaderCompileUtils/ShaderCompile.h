#pragma once
#define _TCHAR_DEFINED
#include <d3dcommon.h>
#include "ShaderCompileData.h"

struct FFileMetadata;


/**
 * HLSL 셰이더를 컴파일할 때 include 파일을 확인하는 데 사용됩니다.
 * 
 * 모든 포함된 파일 및 해당 확인 경로에 대한 메타데이터를 저장하여 추적 및 추가 처리를 수행합니다.
 */
class FShaderIncludeHandler : public ID3DInclude
{
public:
    FShaderIncludeHandler() = default;
    virtual ~FShaderIncludeHandler() = default;

    FShaderIncludeHandler(const FShaderIncludeHandler&) = delete;
    FShaderIncludeHandler& operator=(const FShaderIncludeHandler&) = delete;
    FShaderIncludeHandler(FShaderIncludeHandler&&) = delete;
    FShaderIncludeHandler& operator=(FShaderIncludeHandler&&) = delete;

    [[nodiscard]] TArray<FFileMetadata>& GetIncludePaths();

protected:
    virtual HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) noexcept override;
    virtual HRESULT Close(LPCVOID pData) noexcept override;

private:
    TArray<FFileMetadata> IncludePaths;
};


class FBackgroundShaderCompile
{
public:
    /** 셰이더를 Background에서 컴파일 합니다. */
    static FShaderCompileResult Compile(const FShaderCompileJob& Job);
};
