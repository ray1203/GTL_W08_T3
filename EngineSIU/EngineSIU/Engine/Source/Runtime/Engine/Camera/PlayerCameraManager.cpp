#include "PlayerCameraManager.h"
#include "Editor/LevelEditor/SLevelEditor.h"
#include "Editor/UnrealEd/EditorViewportClient.h"
#include "Actors/Player.h"
#include "World/World.h"
#include "Camera/CameraComponent.h"
#include "CameraModifier/CameraModifier.h"

APlayerCameraManager::APlayerCameraManager()
{
}

void APlayerCameraManager::UpdateCamera(float DeltaTime)
{
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

    if (bIsFading)
    {
        // Fade 처리 로직
        FadeTimeRemaining -= DeltaTime;
        if (FadeTimeRemaining <= 0.0f)
        {
            bIsFading = false;
            OnFadeEnded.Broadcast();
        }

        FadeTimeRemaining = FMath::Max(FadeTimeRemaining - DeltaTime, 0.0f);
        if (FadeTime > 0.0f)
        {
            FadeAmount = FadeAlpha.X + ((1.f - FadeTimeRemaining / FadeTime) * (FadeAlpha.Y - FadeAlpha.X));
        }
    }
}

UCameraModifier* APlayerCameraManager::AddCameraModifier(UCameraModifier* InModifier)
{
    return nullptr;
}

void APlayerCameraManager::RemoveCameraModifier(UCameraModifier* InModifier)
{
}

void APlayerCameraManager::ClearCameraModifiers()
{

}

void APlayerCameraManager::EnableModifier(UCameraModifier* InModifier)
{
}

void APlayerCameraManager::DisableModifier(UCameraModifier* InModifier)
{
}

void APlayerCameraManager::StartCameraFade(float FromAlpha, float ToAlpha, float Duration, const FLinearColor& Color)
{
    if (bIsFading)
    {
        UE_LOG(ELogLevel::Warning, TEXT("PlayerCameraManager::StartCameraFade : Already Fading!"));
        return;
    }

    if (CurrentViewport == nullptr)
    {
        UE_LOG(ELogLevel::Error, TEXT("PlayerCameraManager::StartCameraFade :  Viewport Not Bound!"));
        return;
    }

    bIsFading = true;
    FadeTimeRemaining = Duration;
    FadeColor = Color;
    FadeAlpha = FVector2D(FromAlpha, ToAlpha);
}

UCameraComponent* APlayerCameraManager::GetCameraComponent()
{
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

    FadeTimeRemaining -= DeltaTime;
    if (FadeTimeRemaining <= 0.0f)
    {
        FadeTimeRemaining = 0.0f;
        bIsFading = false;
        OnFadeEnded.Broadcast();
    }

    // !TODO : 참조중인 Viewport에 대해 관련 Fade 함수 실행 
}
