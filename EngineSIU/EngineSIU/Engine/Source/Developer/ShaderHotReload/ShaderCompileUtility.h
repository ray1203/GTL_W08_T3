#pragma once
#define _TCHAR_DEFINED
#include <d3dcommon.h>

#include "Container/Array.h"
#include "Container/Pair.h"


/**
 * D3D_SHADER_MACRO C-스타일 배열을 TArray<TPair<FString, FString>>으로 변환합니다.
 * @param Defines D3D_SHADER_MACRO 배열의 시작 포인터. nullptr일 수 있습니다.
 * @return 변환된 매크로 정의 TArray.
 */
static TArray<TPair<FString, FString>> ConvertD3DMacrosToFStringPairs(const D3D_SHADER_MACRO* Defines);
