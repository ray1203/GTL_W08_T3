#pragma once
#include "Components/SceneComponent.h"

class FViewportClient;

class UCameraComponent : public USceneComponent
{
    DECLARE_CLASS(UCameraComponent, USceneComponent)

public:
    UCameraComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;

private:
    float ViewFOV = 90.0f;
    float AspectRatio = 1.777f;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;
public:
    float GetFOV() const { return ViewFOV; }
    void SetFOV(float InFOV) { ViewFOV = InFOV; }
    float GetNearClip() const { return NearClip; }
    void SetNearClip(float InNearClip) { NearClip = InNearClip; }
    float GetFarClip() const { return FarClip; }
    void SetFarClip(float InFarClip) { FarClip = InFarClip; }

    FMatrix View;
    FMatrix Projection;

    FMatrix& GetViewMatrix() { return View; }
    FMatrix& GetProjectionMatrix() { return Projection; }

    void UpdateViewMatrix();
    void UpdateProjectionMatrix(float AspectRatio);

    FVector GetCameraLocation() const;
    float GetCameraNearClip() const { return NearClip; }
    float GetCameraFarClip() const { return FarClip; }

    FVector GetForwardVector() const;
    FVector GetRightVector() const;
    FVector GetUpVector() const;
};
