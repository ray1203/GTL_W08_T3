#include "FLuaScriptSystem.h"

#include <filesystem>
#include <tchar.h>

#include "Components/Lua/LuaScriptComponent.h"
#include "GameFramework/Actor.h"
#include "Math/Vector.h"
#include <shellapi.h>


FLuaScriptSystem& FLuaScriptSystem::Get()
{
    static FLuaScriptSystem Instance;
    return Instance;
}

void FLuaScriptSystem::Initialize()
{
    Lua.open_libraries(sol::lib::base, sol::lib::math);

    Lua.new_usertype<FVector>("Vector",
        sol::constructors<FVector(), FVector(float, float, float)>(),
        "x", &FVector::X,
        "y", &FVector::Y,
        "z", &FVector::Z,
        sol::meta_function::addition, [](const FVector& a, const FVector& b) { return a + b; },
        sol::meta_function::multiplication, [](const FVector& a, float f) { return a * f; }
    );
    Lua["Vector"] = [](float x, float y, float z) { return FVector(x, y, z); };
    Lua.new_usertype<FRotator>("Rotator",
        sol::constructors<FRotator(), FRotator(float, float, float)>(),
        "Pitch", &FRotator::Pitch,
        "Yaw", &FRotator::Yaw,
        "Roll", &FRotator::Roll
    );
    Lua.new_usertype<FString>("FString",
        "ToString", [](const FString& Str) { return std::string(TCHAR_TO_UTF8(*Str)); }
    );

    /*Lua.new_usertype<UObject>("UObject",
        "GetName", &UObject::GetName,
        "GetUUID", &UObject::GetUUID
    );*/
    Lua.new_usertype<AActor>("GameObject",
        sol::base_classes, sol::bases<UObject>(),

        "Location",
        sol::property(&AActor::GetActorLocation, &AActor::SetActorLocation),
        "Rotation",
        sol::property(&AActor::GetActorRotation, &AActor::SetActorRotation),
        "Velocity",
        sol::property(&AActor::GetLuaVelocity, &AActor::SetLuaVelocity),
        "UUID",
        sol::readonly_property(&AActor::GetUUID),

        "PrintLocation", [](AActor* Self) {
            auto loc = Self->GetActorLocation();
            UE_LOG(ELogLevel::Display, TEXT("[Lua] Location: %f %f %f"), loc.X, loc.Y, loc.Z);
        },
        "PrintVector",[](AActor* Self, FVector a)
        {
            UE_LOG(ELogLevel::Display, "Print Vector: %f %f %f", a.X, a.Y, a.Z);
        }
    );
    Lua.set_function("print", [](const std::string& msg)
        {
            UE_LOG(ELogLevel::Display, TEXT("[Lua Print] %s"), *FString(msg));
        });



}

FString FLuaScriptSystem::GetScriptFullPath(const FString& ScriptName)
{
    return FString::Printf(TEXT("GameContent/Scripts/%s.lua"), *ScriptName);
}
bool FLuaScriptSystem::CopyTemplateScriptIfNeeded(const FString& SceneName, const FString& ActorName)
{
    FString TargetScript = FString::Printf(TEXT("GameContent/Scripts/%s_%s.lua"), *SceneName, *ActorName);
    FString TemplateScript = TEXT("GameContent/Scripts/template.lua");

    if (std::filesystem::exists(TCHAR_TO_UTF8(*TargetScript)))
    {
        return true; // 이미 존재함
    }

    if (!std::filesystem::exists(TCHAR_TO_UTF8(*TemplateScript)))
    {
        UE_LOG(ELogLevel::Error, TEXT("template.lua가 존재하지 않습니다."));
        return false;
    }

    try
    {
        std::filesystem::copy_file(
            TCHAR_TO_UTF8(*TemplateScript),
            TCHAR_TO_UTF8(*TargetScript),
            std::filesystem::copy_options::overwrite_existing
        );
        return true;
    }
    catch (const std::exception& e)
    {
        UE_LOG(ELogLevel::Error, TEXT("Lua 스크립트 복사 실패: %s"), *FString(e.what()));
        return false;
    }
}

void FLuaScriptSystem::OpenLuaScriptEditor(const FString& ScriptFilePath)
{
    std::string UTF8Path = TCHAR_TO_UTF8(*ScriptFilePath);
    std::filesystem::path AbsolutePath = std::filesystem::absolute(UTF8Path);
    std::wstring WPath = AbsolutePath.wstring(); // ✅ 여기서 .wstring() 명시적으로 사용
    HINSTANCE hInst = ShellExecute(
        NULL, _T("open"), WPath.c_str(), NULL, NULL, SW_SHOWNORMAL
    );

    if ((INT_PTR)hInst <= 32)
    {
        MessageBox(NULL, _T("파일 열기에 실패했습니다."), _T("Lua Script"), MB_OK | MB_ICONERROR);
    }
}
