#include "LuaScriptComponent.h"
#include "Lua/FLuaScriptSystem.h"
#include <filesystem>
#include <fstream>

#include "Classes/Actors/Player.h"
#include "Classes/Components/Shapes/ShapeComponent.h"
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

    /*sol::object VelocityObj = LuaScriptTable["Velocity"];
    if (VelocityObj.is<FVector>())
    {
       FVector a =  VelocityObj.as<FVector>();
       if (a!=FVector::Zero())
       {
           
       }
    }*/
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
        UE_LOG(ELogLevel::Error, TEXT("[Lua] Script not found: %s"), *FullPath);
        return;
    }

    // Lua 전역 상태
    auto& lua = FLuaScriptSystem::Get().GetLuaState();

    // (1) LuaScriptTable 따로 생성
    LuaScriptTable = lua.create_table();

    // (2) environment 생성 (LuaScriptTable을 기반으로)
    sol::environment Env(lua, sol::create, lua.globals());

    // obj 바인딩을 Environment에도 넣어야 함
    if (GetOwner()->IsA<APlayer>())
    {
        Env["obj"] = Cast<APlayer>(GetOwner());
    }
    else
    {
        Env["obj"] = GetOwner();
    }

    // (3) 파일 직접 읽기
    std::ifstream FileStream(UTF8Path);
    std::stringstream Buffer;
    Buffer << FileStream.rdbuf();
    std::string ScriptContent = Buffer.str();

    // (4) 스크립트 실행
    sol::protected_function_result Result = lua.safe_script(ScriptContent, Env);

    // (5) 오류 처리
    if (!Result.valid())
    {
        sol::error Err = Result;
        UE_LOG(ELogLevel::Error, TEXT("[Lua] Failed to execute script %s: %s"), *LuaScriptPath, Err.what());
        LuaScriptTable = sol::nil;
        return;
    }

    // (6) LuaScriptTable을 Env로 다시 덮어쓰기
    LuaScriptTable = Env;

}

void ULuaScriptComponent::SetScriptPath(const FString& InPath)
{
    LuaScriptPath = InPath;
    LoadLuaScript(); //변경 즉시 Lua 스크립트 재로드
}



void ULuaScriptComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{

    TMap<FString, FString>& Properties = OutProperties;

    Properties.Add(TEXT("LuaScriptPath"), *GetScriptPath());

}

void ULuaScriptComponent::SetProperties(const TMap<FString, FString>& Properties)
{
    const FString* TempStr = nullptr;

    // --- 설정 값 복원 ---

    TempStr = Properties.Find(TEXT("LuaScriptPath")); // bAutoActive 변수가 있다고 가정
    if (TempStr)
    {
        SetScriptPath(GetData(*TempStr));
    }
}
