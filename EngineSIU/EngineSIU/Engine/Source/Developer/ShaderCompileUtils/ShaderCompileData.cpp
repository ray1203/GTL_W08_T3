#include "ShaderCompileData.h"
#include <cassert>


const char* ShaderTypeToTargetProfile(EShaderType ShaderType)
{
    switch (ShaderType)
    {
    case EShaderType::VertexShader:
        return "vs_5_0";
    case EShaderType::PixelShader:
        return "ps_5_0";
    default:  // NOLINT(clang-diagnostic-covered-switch-default)
        assert(false && "Unknown shader type");
        return nullptr;
    }
}
