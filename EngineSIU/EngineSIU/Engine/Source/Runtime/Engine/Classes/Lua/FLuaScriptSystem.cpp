#include "FLuaScriptSystem.h"

#include <filesystem>
#include <tchar.h>

#include "Components/Lua/LuaScriptComponent.h"
#include "GameFramework/Actor.h"
#include "Classes/Actors/Player.h"
#include "Math/Vector.h"
#include <shellapi.h>

#include "FInputManager.h"
#include "Components/UI/UUIButtonComponent.h"
#include "Components/UI/UUITextComponent.h"
#include "Engine/Engine.h"
#include "World/World.h"
#include "Engine/StaticMeshActor.h" 
#include "Components/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h" 
#include "Components/Shapes/SphereComponent.h"
#include "Engine/EditorEngine.h"
#include "Actors/Projectile.h" // Add this include to resolve "AProjectile" identifier
#include "Actors/RiceMonkey.h"
#include "Engine/GameEngine.h"
#include <Actors/GameManager.h>
#include <Sound/SoundManager.h>

#include "Components/UI/UUIPanelComponent.h"
#include <WindowsCursor.h>
#include <Camera/PlayerCameraManager.h>


FLuaScriptSystem& FLuaScriptSystem::Get()
{
    static FLuaScriptSystem Instance;
    return Instance;
}

void FLuaScriptSystem::Initialize()
{
    Lua.open_libraries(sol::lib::base, sol::lib::math);
    BindTypes();
    BindActor();
    BindInput();
    BindUtilities();
    BindUI();

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
void FLuaScriptSystem::BindTypes()
{
    Lua.new_usertype<FVector>("Vector",
        sol::constructors<FVector(), FVector(float, float, float)>(),
        "x", &FVector::X,
        "y", &FVector::Y,
        "z", &FVector::Z,
        "Normalize", [](const FVector& Vec) {return Vec.GetSafeNormal(); },
        "Length", [](const FVector& Vec) -> float { return Vec.Length(); },
        sol::meta_function::addition, [](const FVector& a, const FVector& b) { return a + b; },
        sol::meta_function::subtraction, [](const FVector& a, const FVector& b) { return a - b; },
        sol::meta_function::multiplication, [](const FVector& a, float f) 
		{ 
			return a * f; 
		}
    );
    Lua["Vector"] = [](float x, float y, float z) { return FVector(x, y, z); };
    /* Lua.set_function("CreateVector", [](float x, float y, float z) {
         return FVector(x, y, z);
         });*/

    Lua.new_usertype<FRotator>("Rotator",
        sol::constructors<FRotator(), FRotator(float, float, float)>(),
        "Pitch", &FRotator::Pitch,
        "Yaw", &FRotator::Yaw,
        "Roll", &FRotator::Roll
    );

    Lua["Rotator"] = [](float Pitch, float Yaw, float Roll) {return FRotator(Pitch, Yaw, Roll); };

    Lua.new_usertype<FString>("FString",
        "ToString", [](const FString& Str) { return std::string(TCHAR_TO_UTF8(*Str)); }
    );

    Lua.new_usertype<FLinearColor>("LinearColor",
        sol::constructors<FLinearColor(), FLinearColor(float, float, float, float)>(),
        "r", &FLinearColor::R,
        "g", &FLinearColor::G,
        "b", &FLinearColor::B,
        "a", &FLinearColor::A,
        sol::meta_function::addition, [](const FLinearColor& a, const FLinearColor& b) { return a + b; },
        sol::meta_function::subtraction, [](const FLinearColor& a, const FLinearColor& b) { return a - b; },
        sol::meta_function::multiplication, [](const FLinearColor& a, float f)
        {
            return a * f;
        }
    );
    Lua.set_function("LinearColor", [](float r, float g, float b, float a) { return FLinearColor(r, g, b, a); });
}

void FLuaScriptSystem::BindActor()
{
    Lua.new_usertype<AActor>("GameObject",
        sol::base_classes, sol::bases<UObject>(),

        "Location", sol::property(&AActor::GetActorLocation, &AActor::SetActorLocation),
        "Rotation", sol::property(&AActor::GetActorRotation, &AActor::SetActorRotation),
        "Scale", sol::property(&AActor::GetActorScale, &AActor::SetActorScale),

        "ForwardVector", sol::property(&AActor::GetActorForwardVector),
        "RightVector", sol::property(&AActor::GetActorRightVector),

        "PlayerPosition", sol::property(&AActor::GetPlayerLocation),
        "UUID", sol::readonly_property(&AActor::GetUUID),

        "PrintLocation", [](AActor* Self) {
            auto loc = Self->GetActorLocation();
            //UE_LOG(ELogLevel::Display, TEXT("[Lua] Location: %f %f %f"), loc.X, loc.Y, loc.Z);
        },
        "Move", [](AActor* Self, FVector Direction, float Scalar)
        {
            if (USceneComponent* RootComp = Self->GetRootComponent())
            {
                RootComp->Translate(Direction * Scalar);
            }
        },
        "Instantiate", [](AActor* Self, const std::string& TypeName, const FVector& StartPos)
        {
            // 여기에 추가
            UWorld* World = Self->GetWorld();
            if (World)
            {
                if (TypeName == "Projectile")
                {
                    AProjectile* NewActor = World->SpawnActor<AProjectile>();
                    NewActor->SetInitialSpeed(Self->GetActorForwardVector() * 100);
                    NewActor->SetActorLocation(StartPos);
                }
            }
        },
        "ShootProjectile", [](AActor* Self, const FVector& ShootPos, const FVector& TargetPos, float Speed)
        {
            UWorld* World = Self->GetWorld();
            if (World)
            {
                AProjectile* NewActor = World->SpawnActor<AProjectile>();
                if (NewActor)
                {
                    FVector Dir = (TargetPos - ShootPos).GetSafeNormal();
                    NewActor->SetInitialSpeed(Dir * Speed + FVector(0,0,0.3));
                    NewActor->SetActorLocation(ShootPos);
                    NewActor->GetComponentByClass<USphereComponent>()->Mass = 1.0f;
                }
            }

        },
        "Velocity", sol::property(&AActor::GetVelocity),
        "Destroy", &AActor::Destroy,
        "GetPlayerCameraManager" , [](AActor* Self) -> APlayerCameraManager*
        {
            UWorld* World = Self->GetWorld();
            if(World)
            {
                ULevel* Level = World->GetActiveLevel();
                if (Level)
                {
                    for (AActor* Actor : Level->Actors)
                    {
                        if (Actor->IsA<APlayerCameraManager>())
                        {
                            APlayerCameraManager* PlayerCameraManager = Cast<APlayerCameraManager>(Actor);
                            return PlayerCameraManager;
                        }
                    }
                }
            }

            return nullptr;
        }
    );

    Lua.new_usertype<APlayer>("APlayer",
        sol::base_classes, sol::bases<AActor>(),
        "Velocity", sol::property(&APlayer::GetVelocity, &APlayer::SetVelocity),
        "Acceleration", sol::property(&APlayer::GetAcceleration, &APlayer::SetAcceleration),
        "bGrounded", sol::readonly_property(&APlayer::IsGrounded),
        "bInputBlock", sol::property(&APlayer::bInputBlock)
    );

	Lua.new_usertype<AGameManager>("AGameManager",
		sol::base_classes, sol::bases<AActor>(),
		"InitGameWorld", &AGameManager::InitGameWorld,
		"InitiateActor", &AGameManager::InitiateActor
		);

    Lua.new_usertype<ARiceMonkey>("ARiceMonkey",
        sol::base_classes, sol::bases<AActor>(),

        // 멤버 변수 바인딩 (읽기/쓰기)
        "bIsAngry", &ARiceMonkey::bIsAngry,
        "SsalMass", &ARiceMonkey::SsalMass,
        "AttackSpeed", &ARiceMonkey::AttackSpeed,
        "HP", &ARiceMonkey::HP,
        "Color", &ARiceMonkey::Color
    );

	Lua.new_usertype<APlayerCameraManager>("APlayerCameraManager",
		sol::base_classes, sol::bases<AActor>(),

		"StartCameraTransition", & APlayerCameraManager::Lua_StartCameraTransition,
		"StartCameraShake", &APlayerCameraManager::StartCameraShake,
		"StartCameraFade", &APlayerCameraManager::StartCameraFade,
        "StartCameraLetterBox", & APlayerCameraManager::StartCameraLetterBox);
}

void FLuaScriptSystem::BindInput()
{
    Lua.new_usertype<FInputKeyManager>("Input", sol::no_constructor,
        "GetKey", &FInputKeyManager::GetKey,
        "GetKeyDown", &FInputKeyManager::GetKeyDown,
        "GetKeyUp", &FInputKeyManager::GetKeyUp
    );
    Lua["Input"] = &FInputKeyManager::Get();

    Lua.new_enum("EKeys",
        "A", EKeys::A,
        "B", EKeys::B,
        "C", EKeys::C,
        "D", EKeys::D,
        "E", EKeys::E,
        "F", EKeys::F,
        "G", EKeys::G,
        "H", EKeys::H,
        "I", EKeys::I,
        "J", EKeys::J,
        "K", EKeys::K,
        "L", EKeys::L,
        "M", EKeys::M,
        "N", EKeys::N,
        "O", EKeys::O,
        "P", EKeys::P,
        "Q", EKeys::Q,
        "R", EKeys::R,
        "S", EKeys::S,
        "T", EKeys::T,
        "U", EKeys::U,
        "V", EKeys::V,
        "W", EKeys::W,
        "X", EKeys::X,
        "Y", EKeys::Y,
        "Z", EKeys::Z,
        "SpaceBar", EKeys::SpaceBar,
        "Esc", EKeys::Escape,
        "LeftMouseButton", EKeys::LeftMouseButton,
        "RightMouseButton", EKeys::RightMouseButton
    );
}

void FLuaScriptSystem::BindUtilities()
{
    Lua.set_function("print", [](const std::string& msg) {
        UE_LOG(ELogLevel::Display, TEXT("[Lua Print] %s"), *FString(msg));
        });
    Lua.set_function("FindActorByLabel", [](const std::string& Label) -> AActor*
        {
            return FindActorByLabel(FString(Label));
        });
    //ActorComp로 가져와도 Lua에서는 형변환/하위 객체에 접근 불가
    Lua.set_function("GetComponentByType", [](AActor* Actor, const std::string& TypeName) -> UActorComponent*
        {
            return GetComponentByTypeName(Actor, TypeName);
        });
    // 새로 추가
    Lua.set_function("GetUITextComponent", [](AActor* Actor) -> UUITextComponent*
        {
            return Actor ? Actor->GetComponentByClass<UUITextComponent>() : nullptr;
        });

    Lua.set_function("GetUIButtonComponent", [](AActor* Actor) -> UUIButtonComponent*
        {
            return Actor ? Actor->GetComponentByClass<UUIButtonComponent>() : nullptr;
        });
    Lua.set_function("GetUIPanelComponent", [](AActor* Actor) -> UUIPanelComponent*
        {
            return Actor ? Actor->GetComponentByClass<UUIPanelComponent>() : nullptr;
        });
    Lua.set_function("RestartGame", []()
        {
                GEngine->bRestartGame = true;
        });
    Lua.set_function("LoadScene", [](int Index)
        {
            if (UGameEngine* GameEngine = Cast<UGameEngine>(GEngine))
            {
                GameEngine->LoadScene(Index);
            }
        });
    Lua.set_function("ExitGame", []()
        {
            PostQuitMessage(0); // Windows 메시지 루프에서 종료 메시지 발생
        });
    Lua.set_function("PlaySFX", [](const std::string& sfxName)
        {
            FSoundManager::Instance().PlaySFX(FString(sfxName), 10);
        });
    Lua.set_function("GameOver", [](APlayer* Player)
        {
            UWorld* World = Player->GetWorld();
            for (auto& actor : World->GetActiveLevel()->Actors)
            {
                UUIComponent* comp = actor->GetComponentByClass<UUIComponent>();
                if (comp)
                {
                    comp->bVisible = true;
                }
            }
            FWindowsCursor::SetShowMouseCursor(true);
            Player->bInputBlock = true;
        });
    Lua.set_function("CameraFade", [](float FromAlpha, float ToAlpha, float Duration, const FLinearColor& Color, bool Override)
        {
            if (APlayerCameraManager* cameraManager = GEngine->ActiveWorld->GetPlayerCameraManager())
            {
                cameraManager->StartCameraFade(FromAlpha, ToAlpha, Duration, Color, Override);

                //cameraManager->StartCameraShake(TEXT("test"), static_cast<uint32>(-1), 0.3f, result.Velocity.Length() / 100);

            }
        });
    Lua.set_function("CameraShake", [](const std::string& CurveName, uint32 TargetPropertiesMask, float Duration, float Amplitude)
        {
            if (APlayerCameraManager* cameraManager = GEngine->ActiveWorld->GetPlayerCameraManager())
            {
                cameraManager->StartCameraShake(CurveName, TargetPropertiesMask, Duration, Amplitude);
            }
        });
}
void FLuaScriptSystem::BindUI()
{
    // Base UIComponent 먼저 바인딩
    Lua.new_usertype<UUIComponent>("UIComponent",
        sol::base_classes, sol::bases<UActorComponent>(),

        "IsVisible", sol::property(
            [](UUIComponent* Comp) { return Comp->bVisible; },
            [](UUIComponent* Comp, bool b) { Comp->bVisible = b; }),

        "Anchor", sol::property(
            [](UUIComponent* Comp) { return static_cast<int>(Comp->Anchor); },
            [](UUIComponent* Comp, int NewAnchor) { Comp->Anchor = static_cast<EUIAnchor>(NewAnchor); }),

        "Offset", sol::property(
            [](UUIComponent* Comp) { return FVector2D(Comp->Offset.X, Comp->Offset.Y); },
            [](UUIComponent* Comp, FVector2D Value) { Comp->Offset = Value; }),

        "Size", sol::property(
            [](UUIComponent* Comp) { return FVector2D(Comp->Size.X, Comp->Size.Y); },
            [](UUIComponent* Comp, FVector2D Value) { Comp->Size = Value; }),

        "bNoBackground", sol::property(
            [](UUIComponent* Comp) { return Comp->bNoBackground; },
            [](UUIComponent* Comp, bool b) { Comp->bNoBackground = b; })
    );

    // UITextComponent
    Lua.new_usertype<UUITextComponent>("UIText",
        sol::base_classes, sol::bases<UUIComponent>(),

        "Text", sol::property(
            [](UUITextComponent* Comp) { return std::string(Comp->GetText()); },
            [](UUITextComponent* Comp, const std::string& Value) { Comp->SetText(Value); }),

        "SetTextColor", [](UUITextComponent* Comp, float r, float g, float b, float a = 1.0f)
        {
            Comp->TextColor = FLinearColor(r, g, b, a);
        },
        "SetTextScale", [](UUITextComponent* Comp, float scale)
        {
            Comp->TextScale = scale;
        }
    );

    // UIButtonComponent
    Lua.new_usertype<UUIButtonComponent>("UIButton",
        sol::base_classes, sol::bases<UUIComponent>(),

        "Label", sol::property(
            [](UUIButtonComponent* Comp) { return std::string(Comp->GetLabel()); },
            [](UUIButtonComponent* Comp, const std::string& Value) { Comp->SetLabel(Value); }),

        "Bind", &UUIButtonComponent::BindLuaCallback
    );
    // UIButtonComponent
    Lua.new_usertype<UUIPanelComponent>("UIPanel",
        sol::base_classes, sol::bases<UUIComponent>()
    );
}


//Lua 내부에서 사용할 함수들
AActor* FLuaScriptSystem::FindActorByLabel(const FString& Label)
{
    UWorld* World = GEngine->ActiveWorld;
    if (!World) return nullptr;

    for (AActor* Actor : World->GetActiveLevel()->Actors)
    {
        if (Actor && Actor->GetActorLabel() == Label)
        {
            return Actor;
        }
    }

    return nullptr;
}
UActorComponent* FLuaScriptSystem::GetComponentByTypeName(AActor* Actor, const std::string& TypeName)
{
    if (!Actor) return nullptr;

    for (UActorComponent* Comp : Actor->GetComponents())
    {
        if (Comp && TCHAR_TO_UTF8(*Comp->GetClass()->GetName()) == TypeName)
        {
            return Comp;
        }
    }

    return nullptr;
}
