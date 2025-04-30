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

    FLinearColor FadeColor; // fade가 되는 색상
    float FadeAmount; // 현재 fade 얼마나되있는지
    FVector2D FadeAlpha; // 현재 실행되는 fade 효과의 시작 끝 정도
    float FadeTime; // 현재 실행되는 fade 효과의 시간
    float FadeTimeRemaining; // 남은 fade 효과의 시간
    float LetterBoxRatio;

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
