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

    FLinearColor FadeColor = FLinearColor(1,1,1,1); // fade가 되는 색상
    float FadeAmount = 0.f; // 현재 fade 얼마나되있는지
    FVector2D FadeAlpha = FVector2D(0,1); // 현재 실행되는 fade 효과의 시작 끝 정도
    float FadeTime = 0.f; // 현재 실행되는 fade 효과의 시간
    float FadeTimeRemaining = 0.f; // 남은 fade 효과의 시간
    float LetterBoxRatio = 0.f;
    FLinearColor LetterBoxColor = FLinearColor(0, 0, 0, 1);

    FName CameraStyle; // ?
    //struct FViewTarget ViewTarget; -> 뷰포트 직접 참조로 대체

    void UpdateCamera(float DeltaTime);
    UCameraModifier* AddCameraModifier(UCameraModifier* InModifier);
    void RemoveCameraModifier(UCameraModifier* InModifier);
    void ClearCameraModifiers();

    void EnableModifier(UCameraModifier* InModifier);
    void DisableModifier(UCameraModifier* InModifier);

    UCameraComponent* GetCameraComponent();
    void SetCameraComponent(UCameraComponent* InCameraComponent);

public:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual UObject* Duplicate(UObject* InOuter) override;

    // sol bind functions
public:
    // !TODO : shake camera curve이름 파라미터로 넘겨서 실행
    // void ShakeCamera
    void Lua_StartCameraTransition(const FVector& EndPos, const FRotator& EndRot, float TargetFOV, float Duration);
    void StartCameraShake(const FString& CurveName, uint32 TargetPropertiesMask, float Duration, float Amplitude);
    void StartCameraFade(float FromAlpha, float ToAlpha, float Duration, const FLinearColor& Color, bool Override);
    // InType : TransitionIn(0), TransitionOut(1), TransitionNone(2)
    void StartCameraLetterBox(FLinearColor InBoxColor, float InRatio, float InDuration, bool InTransitionByAlpha, bool InTransitionByMove, uint8 InType);

protected:
    virtual void ApplyCameraParams(const FCameraParams& InParams);
    void UpdateFade(float DeltaTime);
    FEditorViewportClient* CurrentViewport;
    UCameraComponent* CurrentCameraComponent = nullptr;

private:
    bool bIsFading = false;
    bool bOverrideFade = false;
    TArray<UCameraModifier*> ModifierList;


public:
    FOnFadeEndedDelegate OnFadeEnded;
};
