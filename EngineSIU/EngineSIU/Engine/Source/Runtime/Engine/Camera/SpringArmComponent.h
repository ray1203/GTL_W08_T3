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

protected:
    void OnRawMouseInput(const FPointerEvent& InEvent);
    void HandleRotation(const FVector2D Vector);

protected:
	float TargetArmLength = 300.0f; // 카메라와의 거리
	FVector SocketOffset = FVector(0, 50.0f, 50.0f); //  캐릭터 기준 오프셋
	FVector TargetLocation; // 최종 카메라 위치

	UCameraComponent* Camera = nullptr;

	// Collision은 안함
private:
    std::optional<FDelegateHandle> MouseInputHandle;
};
