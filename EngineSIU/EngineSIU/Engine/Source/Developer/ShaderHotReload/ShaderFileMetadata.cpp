#include "ShaderFileMetadata.h"
#include <utility>

#include "ShaderCompileUtility.h"

namespace fs = std::filesystem;


FShaderFileMetadata::FShaderFileMetadata(
    FString InEntryPoint,
    const fs::path& InPath,
    TArray<FFileMetadata> InIncludePaths,
    const D3D_SHADER_MACRO* InDefines
)
    : EntryPoint(std::move(InEntryPoint))
    , Defines(ConvertD3DMacrosToFStringPairs(InDefines))
    , FileMetadata(InPath, fs::last_write_time(InPath))
    , IncludePaths(std::move(InIncludePaths))
{
}

bool FShaderFileMetadata::IsOutdatedAndUpdateLastTime(bool bIsCheckIncludes)
{
    // 원본 .hlsl 파일이 바뀌었는지 검사
    const fs::file_time_type NewLastWriteTime = fs::last_write_time(FileMetadata.FilePath);
    if (FileMetadata.LastWriteTime != NewLastWriteTime)
    {
        FileMetadata.LastWriteTime = NewLastWriteTime;
        return true;
    }

    // .hlsl에 포함된 Includes 까지 검사
    if (bIsCheckIncludes)
    {
        for (auto& Include : IncludePaths)
        {
            const fs::file_time_type NewIncludeLastWriteTime = fs::last_write_time(Include.FilePath);
            if (Include.LastWriteTime != NewIncludeLastWriteTime)
            {
                Include.LastWriteTime = NewIncludeLastWriteTime;
                return true;
            }
        }
    }
    return false;
}
