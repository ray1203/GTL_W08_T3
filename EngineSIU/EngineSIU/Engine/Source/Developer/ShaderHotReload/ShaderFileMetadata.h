#pragma once
#include <filesystem>

#include "Container/Array.h"
#include "Container/Pair.h"
#include "Container/String.h"


/**
 * 파일 경로와 마지막 수정 시간을 저장하는 구조체
 */
struct FFileMetadata
{
    FFileMetadata(std::filesystem::path InPath, const std::filesystem::file_time_type& InLastWriteTime)
        : FilePath(std::move(InPath))
        , LastWriteTime(InLastWriteTime)
    {
    }

    std::filesystem::path FilePath;
    std::filesystem::file_time_type LastWriteTime;
};

/**
 * 원본 Shader 파일의 정보를 담고있는 구조체
 */
struct FShaderFileMetadata
{
    FShaderFileMetadata(FString InEntryPoint, const std::filesystem::path& InPath, TArray<FFileMetadata> InIncludePaths);

    /**
     * .hlsl파일이 변경되었는지 확인하고, 마지막으로 수정된 시간을 갱신합니다.
     * @param bIsCheckIncludes .hlsl의 Include 목록까지 검사할지 여부
     * @return 파일이 변경되었는지 여부
     */
    bool IsOutdatedAndUpdateLastTime(bool bIsCheckIncludes = true);

    // 셰이더 호출 시작지점
    FString EntryPoint;

    // 셰이더 Defines
    TArray<TPair<FString, FString>> Defines;

    // 파일 경로, 마지막으로 수정된 시간
    FFileMetadata FileMetadata;

    // .hlsl에 include된 파일 경로 목록
    TArray<FFileMetadata> IncludePaths;
};
