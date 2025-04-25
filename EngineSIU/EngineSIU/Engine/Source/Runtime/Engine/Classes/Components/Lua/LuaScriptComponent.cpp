#include "LuaScriptComponent.h"
#include "Lua/FLuaScriptSystem.h"
#include <filesystem>

#include "GameFramework/Actor.h"

ULuaScriptComponent::ULuaScriptComponent()
{
    bAutoActive = true;
}

void ULuaScriptComponent::BeginPlay()
{
    Super::BeginPlay();
    if (AActor* Owner = GetOwner())
    {
        Owner->SetLuaComponent(this);
    }
    LoadLuaScript();
    CallLuaFunction("BeginPlay");
}

void ULuaScriptComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CallLuaFunction("EndPlay");
    Super::EndPlay(EndPlayReason);
}

void ULuaScriptComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    CallLuaFunction("Tick", DeltaTime);
}
UObject* ULuaScriptComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewLuaComp = Cast<ThisClass>(Super::Duplicate(InOuter));

    // Lua 경로도 복사
    NewLuaComp->LuaScriptPath = LuaScriptPath;

    // 스크립트 테이블은 런타임에 다시 로드되므로 복사 안 함
    // 즉, LoadLuaScript()를 BeginPlay 시점에 다시 호출할 것

    return NewLuaComp;
}

void ULuaScriptComponent::LoadLuaScript()
{
    if (LuaScriptPath.IsEmpty()) return;

    FString FullPath = FLuaScriptSystem::GetScriptFullPath(LuaScriptPath);
    std::string UTF8Path = TCHAR_TO_UTF8(*FullPath);

    if (!std::filesystem::exists(UTF8Path))
    {
        UE_LOG(ELogLevel::Error, *FString::Printf(TEXT("Lua script not found: %s"), *FullPath));
        return;
    }

    auto& lua = FLuaScriptSystem::Get().GetLuaState();
    lua["obj"] = GetOwner();
    lua.script_file(UTF8Path);

    LuaScriptTable = lua.globals();
}

void ULuaScriptComponent::CallLuaFunction(const char* FunctionName, float DeltaTime, AActor* Other)
{
    if (!LuaScriptTable.valid())
    {
        UE_LOG(ELogLevel::Warning, TEXT("LuaScriptTable is invalid for Actor %s"), *GetOwner()->GetActorLabel());
        return;
    }

    sol::function Func = LuaScriptTable[FunctionName];
    if (!Func.valid())
    {
        UE_LOG(ELogLevel::Warning, TEXT("Lua function '%s' not found in script: %s"), *FString(FunctionName), *LuaScriptPath);
        return;
    }

    sol::protected_function_result Result;
    if (strcmp(FunctionName, "Tick") == 0)
        Result = Func(DeltaTime);
    else if (strcmp(FunctionName, "OnOverlap") == 0)
        Result = Func(Other);
    else
        Result = Func();

    if (!Result.valid())
    {
        sol::error err = Result;
        UE_LOG(ELogLevel::Error, *FString::Printf(TEXT("Lua error in %s: %s"), *FString(FunctionName), err.what()));
    }
}
