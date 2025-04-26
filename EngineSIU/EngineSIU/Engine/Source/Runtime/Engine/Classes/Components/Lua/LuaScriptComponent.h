#pragma once

#include <sol/table.hpp>
#include "Components/ActorComponent.h"
#include "UObject/ObjectMacros.h"

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
    void CallLuaFunction(const char* FunctionName, float DeltaTime = 0.0f, AActor* Other = nullptr);
    void LoadLuaScript();

    /** Lua 스크립트 경로 지정 */
    void SetScriptPath(const FString& Path) { LuaScriptPath = Path; }
    const FString& GetScriptPath() const { return LuaScriptPath; }
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const;
    virtual void SetProperties(const TMap<FString, FString>& Properties);


private:
    FVector Velocity;

public:
    const FVector& GetVelocity() const { return Velocity; }
    void SetVelocity(const FVector& InVelocity) { Velocity = InVelocity; }
protected:
    FString LuaScriptPath = TEXT("template");
    sol::table LuaScriptTable;
private:
};
