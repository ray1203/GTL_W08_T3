#include "CameraComponent.h"
#include <Math/JungleMath.h>
#include "UnrealEd/EditorViewportClient.h"
#include "Editor/LevelEditor/SLevelEditor.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"


UCameraComponent::UCameraComponent()
{
}

UObject* UCameraComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->ViewFOV = ViewFOV;
    NewComponent->AspectRatio = AspectRatio;
    NewComponent->NearClip = NearClip;
    NewComponent->FarClip = FarClip;    
    return NewComponent;
}

void UCameraComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);

    // 카메라의 FOV, AspectRatio, NearClip, FarClip을 OutProperties에 추가
    OutProperties.Add(TEXT("FOV"), FString::Printf(TEXT("%f"), ViewFOV));
    OutProperties.Add(TEXT("AspectRatio"), FString::Printf(TEXT("%f"), AspectRatio));
    OutProperties.Add(TEXT("NearClip"), FString::Printf(TEXT("%f"), NearClip));
    OutProperties.Add(TEXT("FarClip"), FString::Printf(TEXT("%f"), FarClip));
}

void UCameraComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    // 카메라의 FOV, AspectRatio, NearClip, FarClip을 InProperties에서 읽어와 설정
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("FOV"));
    if (TempStr) ViewFOV = FCString::Atof(**TempStr);
    TempStr = InProperties.Find(TEXT("AspectRatio"));
    if (TempStr) AspectRatio = FCString::Atof(**TempStr);
    TempStr = InProperties.Find(TEXT("NearClip"));
    if (TempStr) NearClip = FCString::Atof(**TempStr);
    TempStr = InProperties.Find(TEXT("FarClip"));
    if (TempStr) FarClip = FCString::Atof(**TempStr);
}

void UCameraComponent::UpdateViewMatrix()
{
    View = JungleMath::CreateViewMatrix(GetWorldLocation(),
        GetWorldLocation() + GetForwardVector(),
        FVector{ 0.0f,0.0f, 1.0f }
    );
}

void UCameraComponent::UpdateProjectionMatrix(float AspectRatio)
{
    // 이 카메라가 사용중인 뷰포트 알아내야함
    Projection = JungleMath::CreateProjectionMatrix(
        FMath::DegreesToRadians(ViewFOV),
        AspectRatio,
        NearClip,
        FarClip
    );
}

FVector UCameraComponent::GetCameraLocation() const
{
    return USceneComponent::GetWorldLocation();
}

FVector UCameraComponent::GetForwardVector() const
{
    FVector Forward = FVector(1.f, 0.f, 0.0f);
    Forward = JungleMath::FVectorRotate(Forward, GetWorldRotation());
    return Forward;
}

FVector UCameraComponent::GetRightVector() const
{
    FVector Right = FVector(0.f, 1.f, 0.0f);
    Right = JungleMath::FVectorRotate(Right, GetWorldRotation());
    return Right;
}

FVector UCameraComponent::GetUpVector() const
{
    FVector Up = FVector(0.f, 0.f, 1.0f);
    Up = JungleMath::FVectorRotate(Up, GetWorldRotation());
    return Up;
}

