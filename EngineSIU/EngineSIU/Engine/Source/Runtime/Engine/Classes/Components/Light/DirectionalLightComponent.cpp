#include "DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Math/Rotator.h"
#include "Math/Quat.h"
#include "UObject/Casts.h"

UDirectionalLightComponent::UDirectionalLightComponent()
{
    DirectionalLightInfo.Direction = -USceneComponent::GetUpVector();
    DirectionalLightInfo.Intensity = 10.0f;

    DirectionalLightInfo.LightColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

UObject* UDirectionalLightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->DirectionalLightInfo = DirectionalLightInfo;
    }
    
    return NewComponent;
}

void UDirectionalLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("LightColor"), *DirectionalLightInfo.LightColor.ToString());
    OutProperties.Add(TEXT("Intensity"), FString::Printf(TEXT("%f"), DirectionalLightInfo.Intensity));
    OutProperties.Add(TEXT("Direction"), *DirectionalLightInfo.Direction.ToString());
}

void UDirectionalLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    auto SetPropertyHelper = [&InProperties] <typename T, typename Fn>(const FString& Key, T& MemberVariable, const Fn& ConversionFunc)
    {
        if (const FString* TempStr = InProperties.Find(Key))
        {
            MemberVariable = ConversionFunc(*TempStr);
        }
    };

    SetPropertyHelper(TEXT("LightColor"), DirectionalLightInfo.LightColor, [](const FString& Str) { FLinearColor Color; Color.InitFromString(Str); return Color; });
    SetPropertyHelper(TEXT("Intensity"),  DirectionalLightInfo.Intensity,  [](const FString& Str) { return FString::ToFloat(Str); });
    SetPropertyHelper(TEXT("Direction"),  DirectionalLightInfo.Direction,  [](const FString& Str) { FVector Dir; Dir.InitFromString(Str); return Dir; });
}

FVector UDirectionalLightComponent::GetDirection()  
{
    return GetWorldRotation()
           .ToQuaternion()
           .RotateVector(-GetUpVector());
}

const FDirectionalLightInfo& UDirectionalLightComponent::GetDirectionalLightInfo() const
{
    return DirectionalLightInfo;
}

void UDirectionalLightComponent::SetDirectionalLightInfo(const FDirectionalLightInfo& InDirectionalLightInfo)
{
    DirectionalLightInfo = InDirectionalLightInfo;
}

float UDirectionalLightComponent::GetIntensity() const
{
    return DirectionalLightInfo.Intensity;
}

void UDirectionalLightComponent::SetIntensity(float InIntensity)
{
    DirectionalLightInfo.Intensity = InIntensity;
}

FLinearColor UDirectionalLightComponent::GetLightColor() const
{
    return DirectionalLightInfo.LightColor;
}

void UDirectionalLightComponent::SetLightColor(const FLinearColor& InColor)
{
    DirectionalLightInfo.LightColor = InColor;
}
