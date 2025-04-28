#pragma once

#include <sol/table.hpp>
#include "Components/ActorComponent.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"

class AActor;

class ULuaScriptComponent : public UActorComponent
{
    DECLARE_CLASS(ULuaScriptComponent, UActorComponent)

public:
    ULuaScriptComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime) override;
    virtual UObject* Duplicate(UObject* InOuter) override;

    /** 외부에서 직접 호출할 수 있도록 함수 제공 */
    template<typename... Args>
    void CallLuaFunction(const char* FunctionName, Args&&... args);
    void LoadLuaScript();

    /** Lua 스크립트 경로 지정 */
    void SetScriptPath(const FString& Path);
    const FString& GetScriptPath() const { return LuaScriptPath; }
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& Properties) override;


//private:
//    FVector Velocity;
//
//public:
//    const FVector& GetVelocity() const { return Velocity; }
//    void SetVelocity(const FVector& InVelocity) { Velocity = InVelocity; }
protected:
    FString LuaScriptPath = TEXT("template");
    sol::table LuaScriptTable;
private:
};

template<typename... Args>
void ULuaScriptComponent::CallLuaFunction(const char* FunctionName, Args&&... args)
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

    sol::protected_function_result Result = Func(std::forward<Args>(args)...);

    if (!Result.valid())
    {
        sol::error err = Result;
        UE_LOG(ELogLevel::Error, *FString::Printf(TEXT("Lua error in %s: %s"), *FString(FunctionName), err.what()));
    }
}
