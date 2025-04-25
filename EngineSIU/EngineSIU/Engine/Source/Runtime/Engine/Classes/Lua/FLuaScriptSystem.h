#pragma once
#include <sol/sol.hpp>

#include "HAL/PlatformType.h"

class FString;

class FLuaScriptSystem
{
public:
    void Initialize(); // register types, functions
    sol::state& GetLuaState() { return Lua; }
    static FString GetScriptFullPath(const FString& ScriptName);

    static FLuaScriptSystem& Get();
private:
    sol::state Lua;
};
