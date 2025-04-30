#pragma once
#include "HAL/PlatformType.h"

class FString;

class FFileHelper
{
public:
    static bool LoadFileToString(FString& Result, const TCHAR* Filename);
    static bool SaveStringToFile(const FString& Text, const TCHAR* Filename);
};
