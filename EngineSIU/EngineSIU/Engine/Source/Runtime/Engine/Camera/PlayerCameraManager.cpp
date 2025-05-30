#include "PlayerCameraManager.h"
#include "Editor/LevelEditor/SLevelEditor.h"
#include "Editor/UnrealEd/EditorViewportClient.h"
#include "Actors/Player.h"
#include "World/World.h"
#include "Camera/CameraComponent.h"
#include "CameraModifier/CameraModifier.h"
#include <CameraModifier/CameraModifier_CameraTransition.h>
#include "CameraModifier/CameraModifier_CameraShake.h"
#include "CameraModifier/CameraModifier_LetterBox.h"

APlayerCameraManager::APlayerCameraManager()
{
}

void APlayerCameraManager::UpdateCamera(float DeltaTime)
{
    // 우선순위를 기준으로 먼저 정렬
    auto ComparePriority = [](const UCameraModifier* A, const UCameraModifier* B) {
        return A->GetPriority() > B->GetPriority();
        };

    ModifierList.Sort(ComparePriority);

    if (CurrentCameraComponent == nullptr)
    {
        CurrentCameraComponent = GetWorld()->GetPlayer()->GetComponentByClass<UCameraComponent>();
    }
    for (UCameraModifier* Modifier : ModifierList)
    {
        if (Modifier && Modifier->IsDisabled() == false)
        {
            FCameraParams Params;
            Params.Position = CurrentCameraComponent->GetWorldLocation();
            Params.Rotation = CurrentCameraComponent->GetWorldRotation();
            Params.FOV = CurrentCameraComponent->GetFOV();

            if (Modifier->ModifyCamera(DeltaTime, Params))
            {
                // 카메라 파라미터를 적용하는 로직
                ApplyCameraParams(Params);
            }
        }
    }

}

UCameraModifier* APlayerCameraManager::AddCameraModifier(UCameraModifier* InModifier)
{
    if (InModifier)
    {
        ModifierList.Add(InModifier);
        InModifier->OnAdded();
        return InModifier;
    }

    UE_LOG(ELogLevel::Error, TEXT("PlayerCameraManager::AddCameraModifier : Invalid Modifier!"));
    return nullptr;
}

void APlayerCameraManager::RemoveCameraModifier(UCameraModifier* InModifier)
{
    if (InModifier)
    {
        if (ModifierList.Contains(InModifier))
        {
            ModifierList.Remove(InModifier);
            InModifier->OnRemoved();
            GUObjectArray.MarkRemoveObject(InModifier);
        }
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("PlayerCameraManager::RemoveCameraModifier : Invalid Modifier!"));
    }
}

void APlayerCameraManager::ClearCameraModifiers()
{
    for (UCameraModifier* Modifier : ModifierList)
    {
        if (Modifier)
        {
            Modifier->OnRemoved();
        }
    }
    ModifierList.Empty();
}

void APlayerCameraManager::EnableModifier(UCameraModifier* InModifier)
{
    if (InModifier)
    {
        InModifier->EnableModifier();
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("PlayerCameraManager::EnableModifier : Invalid Modifier!"));
    }
}

void APlayerCameraManager::DisableModifier(UCameraModifier* InModifier)
{
    if (InModifier)
    {
        InModifier->DisableModifier();
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("PlayerCameraManager::DisableModifier : Invalid Modifier!"));
    }
}

void APlayerCameraManager::StartCameraFade(float FromAlpha, float ToAlpha, float Duration, const FLinearColor& Color, bool Override)
{
    //if (bIsFading)
    //{
    //    UE_LOG(ELogLevel::Warning, TEXT("PlayerCameraManager::StartCameraFade : Already Fading!"));
    //    return;
    //}

    if (CurrentViewport == nullptr)
    {
        UE_LOG(ELogLevel::Error, TEXT("PlayerCameraManager::StartCameraFade :  Viewport Not Bound!"));
        return;
    }

    if (bOverrideFade)
    {
        return;
    }

    bIsFading = true;
    bOverrideFade = Override;
    FadeTime = Duration;
    FadeTimeRemaining = FadeTime;
    FadeColor = Color;
    FadeAlpha = FVector2D(FromAlpha, ToAlpha);
}

void APlayerCameraManager::StartCameraLetterBox(FLinearColor InBoxColor, float InRatio, float InDuration, bool InTransitionByAlpha, bool InTransitionByMove, uint8 InType)
{
    UCameraModifier_LetterBox* LetterBoxModifier = FObjectFactory::ConstructObject<UCameraModifier_LetterBox>(GetWorld());
    if (LetterBoxModifier)
    {
        LetterBoxModifier->SetOwner(this);
        LetterBoxModifier->SetLetterBox(InBoxColor, InRatio, InDuration, InTransitionByAlpha, InTransitionByMove, static_cast<UCameraModifier_LetterBox::TransitionType>(InType));
        LetterBoxModifier->EnableModifier();
        LetterBoxModifier->OnLetterBoxEnd.AddLambda([&]() {
            RemoveCameraModifier(LetterBoxModifier);
            });
        AddCameraModifier(LetterBoxModifier);
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("PlayerCameraManager::StartCameraLetterBox : Failed to create LetterBox Modifier!"));
    }
}

UCameraComponent* APlayerCameraManager::GetCameraComponent()
{
    if (!CurrentCameraComponent)
    {
        APlayer* Player = GetWorld()->GetPlayer();
        if (Player)
        {
            CurrentCameraComponent = Player->GetComponentByClass<UCameraComponent>();
        }
        else
        {
            UE_LOG(ELogLevel::Error, TEXT("PlayerCameraManager::GetCameraComponent : No Player Found!"));
        }
    }
    return CurrentCameraComponent;
}

void APlayerCameraManager::SetCameraComponent(UCameraComponent* InCameraComponent)
{
    CurrentCameraComponent = InCameraComponent;
}


void APlayerCameraManager::BeginPlay()
{
    Super::BeginPlay();

    // 현재 활성 뷰포트를 찾아서 할당
    CurrentViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient().get();

    // 월드에서 현재 플레이어의 카메라를 찾아서 할당
    UWorld* World = GetWorld();

    if (World)
    {
        APlayer* Player = World->GetPlayer();
        if (Player)
        {
            CurrentCameraComponent = Player->GetComponentByClass<UCameraComponent>();
        }
    }
}

void APlayerCameraManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateFade(DeltaTime);
    UpdateCamera(DeltaTime);
}

void APlayerCameraManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

UObject* APlayerCameraManager::Duplicate(UObject* InOuter)
{
    ThisClass* NewCameraManager = Cast<ThisClass>(Super::Duplicate(InOuter));
    // 카메라 스타일, 페이드 색상, 페이드 시간 등 복사
    NewCameraManager->CameraStyle = CameraStyle;
    NewCameraManager->FadeColor = FadeColor;
    NewCameraManager->FadeTime = FadeTime;
    return NewCameraManager;
}

void APlayerCameraManager::Lua_StartCameraTransition(const FVector& EndPos, const FRotator& EndRot, float TargetFOV, float Duration)
{
    UCameraModifier_CameraTransition* TransitionModifier = FObjectFactory::ConstructObject<UCameraModifier_CameraTransition>(GetWorld());
    if (TransitionModifier)
    {
        TransitionModifier->SetOwner(this);
        TransitionModifier->SetTransitionTarget(EndPos, EndRot, TargetFOV);
        TransitionModifier->SetTransitionTime(Duration);
        TransitionModifier->EnableModifier();
        TransitionModifier->OnTransitionEnd.AddLambda([&]() {
            RemoveCameraModifier(TransitionModifier);
            });
        AddCameraModifier(TransitionModifier);
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("PlayerCameraManager::StartCameraTransition : Failed to create CameraTransition Modifier!"));
    }
}

void APlayerCameraManager::StartCameraShake(const FString& CurveName, uint32 TargetPropertiesMask, float Duration, float Amplitude)
{
    UCameraModifier_CameraShake* ShakeModifier = FObjectFactory::ConstructObject<UCameraModifier_CameraShake>(GetWorld());
    if (ShakeModifier)
    {
        ShakeModifier->SetOwner(this);
        ShakeModifier->SetShakeCurve(CurveName, TargetPropertiesMask, Duration, Amplitude);
        ShakeModifier->EnableModifier();
        ShakeModifier->OnTransitionEnd.AddLambda([&]() {
            RemoveCameraModifier(ShakeModifier);
            });
        AddCameraModifier(ShakeModifier);
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("PlayerCameraManager::StartCameraShake : Failed to create CameraShake Modifier!"));
    }
}

void APlayerCameraManager::ApplyCameraParams(const FCameraParams& InParams)
{
    if (CurrentCameraComponent)
    {
        CurrentCameraComponent->SetWorldLocation(InParams.Position);
        CurrentCameraComponent->SetWorldRotation(InParams.Rotation);
        CurrentCameraComponent->SetFOV(InParams.FOV);
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("PlayerCameraManager::ApplyCameraParams : No Camera Component!"));
    }
}

void APlayerCameraManager::UpdateFade(float DeltaTime)
{
    if (!bIsFading)
        return;

    FadeTimeRemaining = FMath::Max(FadeTimeRemaining - DeltaTime, 0.0f);

    if (FadeTimeRemaining <= 0.0f)
    {
        FadeTimeRemaining = 0.0f;
        FadeAmount = 0;
        bIsFading = false;
        bOverrideFade = false;
        OnFadeEnded.Broadcast();
        return;
    }
    else
    {
        FadeAmount = FadeAlpha.X + ((1.f - FadeTimeRemaining / FadeTime) * (FadeAlpha.Y - FadeAlpha.X));
    }

}
