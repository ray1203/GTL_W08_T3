#pragma once
#define _TCHAR_DEFINED
#include <d3d11.h>
#include <filesystem>
#include <future>
#include <vector>
#include <mutex>

#include "Container/Map.h"
#include "Container/TSafeQueue.h"
#include "Developer/ShaderCompileUtils/ShaderCompileData.h"
#include "Developer/ShaderCompileUtils/ShaderMetadataPtr.h"


struct FVertexShaderData
{
    ID3DBlob* VertexShaderCSO;
    ID3D11VertexShader* VertexShader;
};

class FDXDShaderManager
{
public:
	FDXDShaderManager() = default;
	FDXDShaderManager(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);

    void ReleaseAllShader();

private:
	ID3D11Device* DXDDevice;
    ID3D11DeviceContext* DefaultDXDeviceContext;

public:
    HRESULT AddVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint);
    HRESULT AddVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines);

    HRESULT AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize);
    HRESULT AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* Defines);

	HRESULT AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint);
    HRESULT AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines);

    HRESULT AddVertexShaderAsync(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint);
    HRESULT AddVertexShaderAsync(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines);

    HRESULT AddVertexShaderAndInputLayoutAsync(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize);
    HRESULT AddVertexShaderAndInputLayoutAsync(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* Defines);

    HRESULT AddPixelShaderAsync(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint);
    HRESULT AddPixelShaderAsync(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines);

    HRESULT WaitForAllShadersAsync();

	ID3D11InputLayout* GetInputLayoutByKey(const std::wstring& Key) const;
	ID3D11VertexShader* GetVertexShaderByKey(const std::wstring& Key) const;
	ID3D11PixelShader* GetPixelShaderByKey(const std::wstring& Key) const;

    void SetVertexShader(const std::wstring& Key, ID3D11DeviceContext* Context = nullptr) const;
    void SetVertexShaderAndInputLayout(const std::wstring& Key, ID3D11DeviceContext* Context = nullptr) const;
    void SetPixelShader(const std::wstring& Key, ID3D11DeviceContext* Context = nullptr) const;

    /** 셰이더를 HotReload 합니다. */
    bool HandleHotReloadShader();

    /** 모든 Vertex Shader의 크기를 가져옵니다. (Byte) */
    uint32 GetTotalVertexShaderSize() const;

    /** 모든 Pixel Shader의 크기를 가져옵니다. (Byte) */
    uint32 GetTotalPixelShaderSize() const;

    
private:
	TMap<std::wstring, ID3D11InputLayout*> InputLayouts;
    TMap<std::wstring, TShaderMetadataPtr<ID3D11VertexShader>> VertexShaders;
	TMap<std::wstring, TShaderMetadataPtr<ID3D11PixelShader>> PixelShaders;

    TSafeQueue<FShaderCompileResult> CompileResultsQueue;

    // 처음 create할때 비동기로 한번에 처리
    std::vector<std::future<HRESULT>> PendingShaderFutures;
    std::mutex FuturesMutex;
};
