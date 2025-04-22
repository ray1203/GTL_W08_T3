#include "ShaderCompileUtility.h"


TArray<TPair<FString, FString>> ConvertD3DMacrosToFStringPairs(const D3D_SHADER_MACRO* Defines)
{
    TArray<TPair<FString, FString>> ResultMacros;

    // 입력 포인터가 유효한지 확인
    if (!Defines)
    {
        return ResultMacros; // 비어있는 배열 반환
    }

    const D3D_SHADER_MACRO* CurrentMacro = Defines;

    // Name이 nullptr이 아닐 때까지 반복 (배열의 끝까지)
    while (CurrentMacro && CurrentMacro->Name != nullptr)
    {
        // LPCSTR (보통 const char*) -> FString (TCHAR*) 변환
        FString MacroName = CurrentMacro->Name;
        FString MacroDefinition;

        // Definition이 nullptr일 수 있습니다. D3DCompile에서는 보통 "1"로 처리됩니다.
        if (CurrentMacro->Definition != nullptr)
        {
            MacroDefinition = CurrentMacro->Definition;
        }
        else
        {
            // Definition이 nullptr이면 "1"로 설정 (D3DCompile의 기본 동작 모방)
            MacroDefinition = TEXT("1");
        }

        // 결과 배열에 Pair로 추가 (Emplace는 효율적)
        ResultMacros.Emplace(MacroName, MacroDefinition);

        // 다음 매크로로 이동
        CurrentMacro++;
    }

    return ResultMacros;
}
