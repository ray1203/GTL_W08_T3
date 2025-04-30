#pragma once
#include "Container/Array.h"
#include "Container/String.h"

namespace StringUtils
{
    void ParseIntoArrayLines(const FString& Source, TArray<FString>& OutLines);
    void ParseIntoArray(const FString& Source, TArray<FString>& OutArray, const FString& Delimiter, bool bCullEmpty = true);
}
