#pragma once
#include "Define.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Camera/PlayerCameraManager.h"

class APlayerCameraManager;

struct FCameraParams
{
    // 카메라의 월드 위치
    FVector Position;
    // 카메라의 월드 회전
    FRotator Rotation;
    float FOV;
};

class UCameraModifier : public UObject
{
    DECLARE_CLASS(UCameraModifier, UObject)

public:
    UCameraModifier();

    virtual bool ModifyCamera(float DeltaTime, FCameraParams& OutParams) { return false; }
    virtual void PostProcess(float DeltaTime) {};

    virtual void EnableModifier() { bDisabled = false; };
    virtual void DisableModifier() { bDisabled = true; };
    bool IsDisabled() const { return bDisabled != 0; }

    virtual void OnAdded() {};
    virtual void OnRemoved() {};

    void SetOwner(APlayerCameraManager* InCameraOwner);

    uint8 GetPriority() const { return Priority; }

protected:
    void UpdateAlpha(float DeltaTime);

    APlayerCameraManager* CameraOwner = nullptr;
    bool bDisabled;
    uint8 Priority;

    float Duration;
    float ElapsedTime = 0.f;

    //// 이 Modifier의 강도. 0이면 적용하지 않고, 1일 때 최대의 효과
    //float Alpha;

    //// Alpha가 0에서 1로 바뀌는 시간
    //float AlphaInTime;

    //// Alpha가 1에서 0으로 바뀌는 시간
    //float AlphaOutTime;
};
