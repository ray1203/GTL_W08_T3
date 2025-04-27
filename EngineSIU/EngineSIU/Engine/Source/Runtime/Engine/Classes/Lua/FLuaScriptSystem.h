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

    static bool CopyTemplateScriptIfNeeded(const FString& SceneName, const FString& ActorName);
    static void OpenLuaScriptEditor(const FString& ScriptFilePath);
private:
    sol::state Lua;
    void BindTypes();
    void BindActor();
    void BindInput();
    void BindUtilities();
    void BindUI();
};
