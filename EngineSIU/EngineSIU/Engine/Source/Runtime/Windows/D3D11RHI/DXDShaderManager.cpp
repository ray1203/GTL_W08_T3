#include "DXDShaderManager.h"
#include <d3dcompiler.h>
#include <fstream>

#include "Container/TSafeQueue.h"
#include "Developer/ShaderCompileUtils/ShaderCompile.h"
#include "UserInterface/Console.h"


FDXDShaderManager::FDXDShaderManager(ID3D11Device* Device)
    : DXDDevice(Device)
{
    VertexShaders.Empty();
    PixelShaders.Empty();
}

void FDXDShaderManager::ReleaseAllShader()
{
    for (auto& [Key, Shader] : VertexShaders)
    {
        if (Shader)
        {
            Shader->Release();
            Shader = nullptr;
        }
    }
    VertexShaders.Empty();

    for (auto& [Key, Shader] : PixelShaders)
    {
        if (Shader)
        {
            Shader->Release();
            Shader = nullptr;
        }
    }
    PixelShaders.Empty();
}

HRESULT FDXDShaderManager::AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    HRESULT hr = S_OK;

    if (DXDDevice == nullptr)
        return S_FALSE;

    ID3DBlob* PsBlob = nullptr;
    ID3DBlob* ErrorBlob = nullptr;
    FShaderIncludeHandler IncludesHandler;
    hr = D3DCompileFromFile(FileName.c_str(), nullptr, &IncludesHandler, EntryPoint.c_str(), "ps_5_0", shaderFlags, 0, &PsBlob, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11PixelShader* NewPixelShader;
    const uint32 ShaderSize = static_cast<uint32>(PsBlob->GetBufferSize());
    hr = DXDDevice->CreatePixelShader(PsBlob->GetBufferPointer(), PsBlob->GetBufferSize(), nullptr, &NewPixelShader);
    if (PsBlob)
        PsBlob->Release();

    if (FAILED(hr))
        return hr;

    PixelShaders[Key] = NewPixelShader;
    PixelShaders[Key].SetMetadata(std::make_unique<FShaderFileMetadata>(EntryPoint, FileName, IncludesHandler.GetIncludePaths(), nullptr));
    PixelShaders[Key].SetShaderSize(ShaderSize);

    return S_OK;
}

HRESULT FDXDShaderManager::AddPixelShader(
    const std::wstring& Key,
    const std::wstring& FileName,
    const std::string& EntryPoint,
    const D3D_SHADER_MACRO* Defines)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    if (DXDDevice == nullptr)
        return S_FALSE;

    ID3DBlob* PsBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    FShaderIncludeHandler IncludesHandler;
    
    HRESULT hr = D3DCompileFromFile(
        FileName.c_str(),
        Defines, // << 매크로 정의 들어가는 핵심 부분
        &IncludesHandler,
        EntryPoint.c_str(),
        "ps_5_0",
        shaderFlags,
        0,
        &PsBlob,
        &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            std::string errMsg((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize());
            std::cerr << "Shader compile error: " << errMsg << std::endl;
            errorBlob->Release();
        }
        return hr;
    }

    ID3D11PixelShader* NewPixelShader;
    const uint32 ShaderSize = static_cast<uint32>(PsBlob->GetBufferSize());
    hr = DXDDevice->CreatePixelShader(PsBlob->GetBufferPointer(), PsBlob->GetBufferSize(), nullptr, &NewPixelShader);

    if (PsBlob)
        PsBlob->Release();

    if (FAILED(hr))
        return hr;

    PixelShaders[Key] = NewPixelShader;
    PixelShaders[Key].SetMetadata(std::make_unique<FShaderFileMetadata>(EntryPoint, FileName, IncludesHandler.GetIncludePaths(), Defines));
    PixelShaders[Key].SetShaderSize(ShaderSize);

    return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint)
{
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    FShaderIncludeHandler IncludesHandler;
    hr = D3DCompileFromFile(FileName.c_str(), nullptr, &IncludesHandler, EntryPoint.c_str(), "vs_5_0", 0, 0, &VertexShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    const uint32 ShaderSize = static_cast<uint32>(VertexShaderCSO->GetBufferSize());
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaders[Key] = NewVertexShader;
    VertexShaders[Key].SetMetadata(std::make_unique<FShaderFileMetadata>(EntryPoint, FileName, IncludesHandler.GetIncludePaths(), nullptr));
    VertexShaders[Key].SetShaderSize(ShaderSize);

    VertexShaderCSO->Release();

    return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShader(
    const std::wstring& Key,
    const std::wstring& FileName,
    const std::string& EntryPoint,
    const D3D_SHADER_MACRO* Defines)
{
    if (DXDDevice == nullptr)
        return S_FALSE;

    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;
    FShaderIncludeHandler IncludesHandler;

    HRESULT hr = D3DCompileFromFile(
        FileName.c_str(),
        Defines,
        &IncludesHandler,
        EntryPoint.c_str(),
        "vs_5_0",
        shaderFlags,
        0,
        &VertexShaderCSO,
        &ErrorBlob);

    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    const uint32 ShaderSize = static_cast<uint32>(VertexShaderCSO->GetBufferSize());
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaders[Key] = NewVertexShader;
    VertexShaders[Key].SetMetadata(std::make_unique<FShaderFileMetadata>(EntryPoint, FileName, IncludesHandler.GetIncludePaths(), Defines));
    VertexShaders[Key].SetShaderSize(ShaderSize);
    VertexShaderCSO->Release();

    return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    FShaderIncludeHandler IncludesHandler;
    hr = D3DCompileFromFile(FileName.c_str(), nullptr, &IncludesHandler, EntryPoint.c_str(), "vs_5_0", shaderFlags, 0, &VertexShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    const uint32 ShaderSize = static_cast<uint32>(VertexShaderCSO->GetBufferSize());
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        return hr;
    }

    ID3D11InputLayout* NewInputLayout;
    hr = DXDDevice->CreateInputLayout(Layout, LayoutSize, VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &NewInputLayout);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaders[Key] = NewVertexShader;
    VertexShaders[Key].SetMetadata(std::make_unique<FShaderFileMetadata>(EntryPoint, FileName, IncludesHandler.GetIncludePaths(), nullptr));
    VertexShaders[Key].SetShaderSize(ShaderSize);
    InputLayouts[Key] = NewInputLayout;

    VertexShaderCSO->Release();

    return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* Defines)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;
    FShaderIncludeHandler IncludesHandler;

    hr = D3DCompileFromFile(FileName.c_str(), Defines, &IncludesHandler, EntryPoint.c_str(), "vs_5_0", shaderFlags, 0, &VertexShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    const uint32 ShaderSize = static_cast<uint32>(VertexShaderCSO->GetBufferSize());
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        return hr;
    }

    ID3D11InputLayout* NewInputLayout;
    hr = DXDDevice->CreateInputLayout(Layout, LayoutSize, VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &NewInputLayout);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaderCSO->Release();

    VertexShaders[Key] = NewVertexShader;
    VertexShaders[Key].SetMetadata(std::make_unique<FShaderFileMetadata>(EntryPoint, FileName, IncludesHandler.GetIncludePaths(), Defines));
    VertexShaders[Key].SetShaderSize(ShaderSize);
    InputLayouts[Key] = NewInputLayout;
    return S_OK;
}

ID3D11InputLayout* FDXDShaderManager::GetInputLayoutByKey(const std::wstring& Key) const
{
    if (InputLayouts.Contains(Key))
    {
        return *InputLayouts.Find(Key);
    }
    return nullptr;
}

ID3D11VertexShader* FDXDShaderManager::GetVertexShaderByKey(const std::wstring& Key) const
{
    if (VertexShaders.Contains(Key))
    {
        return *VertexShaders.Find(Key);
    }
    return nullptr;
}

ID3D11PixelShader* FDXDShaderManager::GetPixelShaderByKey(const std::wstring& Key) const
{
    if (PixelShaders.Contains(Key))
    {
        return *PixelShaders.Find(Key);
    }
    return nullptr;
}

bool FDXDShaderManager::HandleHotReloadShader()
{
    bool bIsHotReloadShader = false;

    FShaderCompileResult CompletedResult;
    while (CompileResultsQueue.Dequeue(CompletedResult))
    {
        ID3DBlob* CsoBlob = CompletedResult.CsoBlob;
        ID3DBlob* ErrorBlob = CompletedResult.ErrorBlob;

        // 셰이더 컴파일 실패시
        if (!CompletedResult.bSuccess)
        {
            if (ErrorBlob)
            {
                UE_LOG(
                    ELogLevel::Error,
                    "[Shader Hot Reload] %s Compile Failed %s",
                    ToString(CompletedResult.Type),
                    ErrorBlob->GetBufferPointer()
                );
                ErrorBlob->Release();
            }
            continue;
        }

        switch (CompletedResult.Type)
        {
        case EShaderType::VertexShader:
        {
            ID3D11VertexShader* NewVertexShader;
            const HRESULT Hr = DXDDevice->CreateVertexShader(
                CsoBlob->GetBufferPointer(),
                CsoBlob->GetBufferSize(),
                nullptr, &NewVertexShader
            );

            // VS 만들기 실패시
            if (FAILED(Hr))
            {
                UE_LOG(ELogLevel::Error, "[Shader Hot Reload] Failed CreateVertexShader");
                CsoBlob->Release();
                break;
            }

            // 기존 셰이더 제거
            auto& Vs = VertexShaders[CompletedResult.ShaderKey];
            Vs->Release();
            
            // 새로운 셰이더 할당
            Vs = NewVertexShader;
            Vs.GetMetadata().IncludePaths = CompletedResult.IncludePaths;
            
            CsoBlob->Release();
            bIsHotReloadShader = true;
            break;
        }
        case EShaderType::PixelShader:
        {
            ID3D11PixelShader* NewPixelShader;
            const HRESULT Hr = DXDDevice->CreatePixelShader(
                CsoBlob->GetBufferPointer(),
                CsoBlob->GetBufferSize(),
                nullptr, &NewPixelShader
            );

            // PS 만들기 실패시
            if (FAILED(Hr))
            {
                UE_LOG(ELogLevel::Error, "[Shader Hot Reload] Failed CreatePixelShader");
                CsoBlob->Release();
                break;
            }

            // 기존 셰이더 제거
            auto& Vs = PixelShaders[CompletedResult.ShaderKey];
            Vs->Release();
            
            // 새로운 셰이더 할당
            Vs = NewPixelShader;
            Vs.GetMetadata().IncludePaths = CompletedResult.IncludePaths;
            
            CsoBlob->Release();
            bIsHotReloadShader = true;
            break;
        }
        default:
            assert(false); // Unreachable
            break;
        }
    }

    for (auto& Vs : VertexShaders)
    {
        FShaderFileMetadata& Data = Vs.Value.GetMetadata();
        if (Data.IsOutdatedAndUpdateLastTime())
        {
            std::thread Th{[&Vs, &Data, &CompileResultsQueue = this->CompileResultsQueue]
            {
                UE_LOG(ELogLevel::Display, "[Shader Hot Reload] Hot Reload %s...", *FString(Vs.Key));
                const FShaderCompileResult CompileResult = FBackgroundShaderCompile::Compile({
                    .ShaderKey = Vs.Key,
                    .FilePath = Data.FileMetadata.FilePath,
                    .Defines = Data.Defines,
                    .EntryPoint = Data.EntryPoint,
                    .Type = EShaderType::VertexShader,
                });
                CompileResultsQueue.Enqueue(CompileResult);
                UE_LOG(ELogLevel::Display, "[Shader Hot Reload] Finish Hot Reload %s", *FString(Vs.Key));
            }};
            Th.detach();
        }
    }

    for (auto& Ps : PixelShaders)
    {
        FShaderFileMetadata& Data = Ps.Value.GetMetadata();
        if (Data.IsOutdatedAndUpdateLastTime())
        {
            std::thread Th{[&Ps, &Data, &CompileResultsQueue = this->CompileResultsQueue]
            {
                UE_LOG(ELogLevel::Display, "[Shader Hot Reload] Hot Reload %s...", *FString(Ps.Key));
                const FShaderCompileResult CompileResult = FBackgroundShaderCompile::Compile({
                    .ShaderKey = Ps.Key,
                    .FilePath = Data.FileMetadata.FilePath,
                    .Defines = Data.Defines,
                    .EntryPoint = Data.EntryPoint,
                    .Type = EShaderType::PixelShader,
                });
                CompileResultsQueue.Enqueue(CompileResult);
                UE_LOG(ELogLevel::Display, "[Shader Hot Reload] Finish Hot Reload %s", *FString(Ps.Key));
            }};
            Th.detach();
        }
    }
    return bIsHotReloadShader;
}
