#include "BackgroundShaderCompile.h"
#include <fstream>
#include <filesystem>

#define _TCHAR_DEFINED
#include <d3dcompiler.h>

namespace fs = std::filesystem;


TArray<FFileMetadata>& FShaderIncludeHandler::GetIncludePaths()
{
    return IncludePaths;
}

HRESULT FShaderIncludeHandler::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) noexcept
{
    const fs::path AbsolutePath = fs::absolute(fs::path("Shaders") / pFileName);
    IncludePaths.Emplace(AbsolutePath, fs::last_write_time(AbsolutePath));

    // 파일 열기
    std::ifstream File(AbsolutePath, std::ios::binary);
    if (!File.is_open())
    {
        return E_FAIL;
    }

    File.seekg(0, std::ios::end);
    const int64 Size = File.tellg();
    File.seekg(0, std::ios::beg);

    char* Data = new char[Size];
    File.read(Data, Size);
    File.close();

    *ppData = Data;
    *pBytes = static_cast<UINT>(Size);

    return S_OK;
}

HRESULT FShaderIncludeHandler::Close(LPCVOID pData) noexcept
{
    delete[] static_cast<const char*>(pData);
    return S_OK;
}

FShaderCompileResult FBackgroundShaderCompile::Compile(const FShaderCompileJob& Job)
{
    FShaderCompileResult Result{};
    Result.Type = Job.Type;

    TArray<D3D_SHADER_MACRO> Defines;
    Defines.Reserve(Job.Defines.Num());
    for (const auto& Define : Job.Defines)
    {
        Defines.Emplace(*Define.Key, *Define.Value);
    }
    Defines.Emplace(nullptr, nullptr);

    UINT ShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    ShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    FShaderIncludeHandler IncludeHandler;
    const HRESULT CompileResult = D3DCompileFromFile(
        Job.FilePath.c_str(),
        Defines.GetData(),
        &IncludeHandler,
        *Job.EntryPoint,
        ShaderTypeToTargetProfile(Job.Type),
        ShaderFlags, 0,
        &Result.CsoBlob,
        &Result.ErrorBlob
    );

    if (SUCCEEDED(CompileResult))
    {
        Result.bSuccess = true;
        Result.IncludePaths = std::move(IncludeHandler.GetIncludePaths());
    }
    else
    {
        Result.bSuccess = false;
    }

    return Result;
}
