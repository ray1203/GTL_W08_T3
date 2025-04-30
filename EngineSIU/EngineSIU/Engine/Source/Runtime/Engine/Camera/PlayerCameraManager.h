#pragma once

#include "GameFramework/Actor.h"

class UCameraModifier;
class FEditorViewportClient;
class UCameraComponent;
struct FCameraParams;


DECLARE_MULTICAST_DELEGATE(FOnFadeEndedDelegate)
class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)
public:
    APlayerCameraManager();

    FLinearColor FadeColor;
    float FadeAmount;
    FVector2D FadeAlpha;
    float FadeTime;
    float FadeTimeRemaining;

    FName CameraStyle; // ?
    //struct FViewTarget ViewTarget; -> 뷰포트 직접 참조로 대체

    void UpdateCamera(float DeltaTime);
    UCameraModifier* AddCameraModifier(UCameraModifier* InModifier);
    void RemoveCameraModifier(UCameraModifier* InModifier);
    void ClearCameraModifiers();

    void EnableModifier(UCameraModifier* InModifier);
    void DisableModifier(UCameraModifier* InModifier);

    void StartCameraFade(float FromAlpha, float ToAlpha, float Duration, const FLinearColor& Color);
    UCameraComponent* GetCameraComponent();
    void SetCameraComponent(UCameraComponent* InCameraComponent);

public:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual UObject* Duplicate(UObject* InOuter) override;

protected:
    virtual void ApplyCameraParams(const FCameraParams& InParams);
    void UpdateFade(float DeltaTime);
    FEditorViewportClient* CurrentViewport;
    UCameraComponent* CurrentCameraComponent;

private:
    bool bIsFading;
    TArray<UCameraModifier*> ModifierList;


public:
    FOnFadeEndedDelegate OnFadeEnded;
};
