#include "FLuaScriptSystem.h"

#include "Components/Lua/LuaScriptComponent.h"
#include "GameFramework/Actor.h"
#include "Math/Vector.h"

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
