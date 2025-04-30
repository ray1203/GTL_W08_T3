#pragma once
#include "Components/SceneComponent.h"
#include <optional>

class UCameraComponent;
struct FPointerEvent;

class USpringArmComponent : public USceneComponent
{
	DECLARE_CLASS(USpringArmComponent, USceneComponent)

public:
	USpringArmComponent();
	virtual UObject* Duplicate(UObject* InOuter) override;
	virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
	virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
	virtual void InitializeComponent() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime) override;
	void SetTargetArmLength(float InLength);
	float GetTargetArmLength() const;
	void SetSocketOffset(const FVector& InOffset);
	FVector GetSocketOffset() const;

    void AttachCamera(UCameraComponent* InCamera);
    void DetachCamera();

    bool IsCameraAttached() const { return bIsCameraAttached; }

protected:
    void OnRawMouseInput(const FPointerEvent& InEvent);
    void HandleRotation(const FVector2D& Vector);
    void UpdateCameraTransform(float DeltaTime);

protected:
	float TargetArmLength = 300.0f; // 카메라와의 거리
	FVector SocketOffset = FVector(0, 50.0f, 50.0f); //  캐릭터 기준 오프셋
	FVector TargetLocation; // 최종 카메라 위치

    float CurrentPitchAngle = 0.0f;

	UCameraComponent* Camera = nullptr;

    bool bIsCameraAttached = false; // 카메라가 Springarm에 붙어있는지 여부
    FVector CachedCameraLocation; // 카메라의 위치
    FRotator CachedCameraRotation; // 카메라의 회전

	// Collision은 안함
private:
    std::optional<FDelegateHandle> MouseInputHandle;
};
