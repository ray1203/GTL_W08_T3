#include "SpotLightComponent.h"
#include "Math/Rotator.h"
#include "Math/Quat.h"
#include "UObject/Casts.h"

USpotLightComponent::USpotLightComponent()
{
    SpotLightInfo.Position = GetWorldLocation();
    SpotLightInfo.Radius = 30.0f;
    SpotLightInfo.Direction = USceneComponent::GetForwardVector();
    SpotLightInfo.LightColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    SpotLightInfo.Intensity = 1000.0f;
    SpotLightInfo.Type = ELightType::SPOT_LIGHT;
    SpotLightInfo.InnerRad = 0.2618f;
    SpotLightInfo.OuterRad = 0.5236f;
    SpotLightInfo.Attenuation = 20.0f;
}

UObject* USpotLightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->SpotLightInfo = SpotLightInfo;
    }
    return NewComponent;
}

void USpotLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);

    OutProperties.Add(TEXT("Position"), SpotLightInfo.Position.ToString());
    OutProperties.Add(TEXT("Radius"), FString::SanitizeFloat(SpotLightInfo.Radius));
    OutProperties.Add(TEXT("Direction"), SpotLightInfo.Direction.ToString());
    OutProperties.Add(TEXT("LightColor"), SpotLightInfo.LightColor.ToString());
    OutProperties.Add(TEXT("Intensity"), FString::SanitizeFloat(SpotLightInfo.Intensity));
    OutProperties.Add(TEXT("Type"), FString::FromInt(SpotLightInfo.Type));
    OutProperties.Add(TEXT("InnerRad"), FString::SanitizeFloat(SpotLightInfo.InnerRad));
    OutProperties.Add(TEXT("OuterRad"), FString::SanitizeFloat(SpotLightInfo.OuterRad));
    OutProperties.Add(TEXT("Attenuation"), FString::SanitizeFloat(SpotLightInfo.Attenuation));
}

void USpotLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    auto SetPropertyHelper = [&InProperties] <typename T, typename Fn>(const FString& Key, T& MemberVariable, const Fn& ConversionFunc)
    {
        if (const FString* TempStr = InProperties.Find(Key))
        {
            MemberVariable = ConversionFunc(*TempStr);
        }
    };

    SetPropertyHelper(TEXT("Position"),    SpotLightInfo.Position,    [](const FString& Str) { FVector Pos; Pos.InitFromString(Str); return Pos; });
    SetPropertyHelper(TEXT("Radius"),      SpotLightInfo.Radius,      [](const FString& Str) { return FString::ToFloat(Str); });
    SetPropertyHelper(TEXT("Direction"),   SpotLightInfo.Direction,   [](const FString& Str) { FVector Dir; Dir.InitFromString(Str); return Dir; });
    SetPropertyHelper(TEXT("LightColor"),  SpotLightInfo.LightColor,  [](const FString& Str) { FLinearColor Color; Color.InitFromString(Str); return Color; });
    SetPropertyHelper(TEXT("Intensity"),   SpotLightInfo.Intensity,   [](const FString& Str) { return FString::ToFloat(Str); });
    SetPropertyHelper(TEXT("Type"),        SpotLightInfo.Type,        [](const FString& Str) { return FString::ToInt(Str); });
    SetPropertyHelper(TEXT("InnerRad"),    SpotLightInfo.InnerRad,    [](const FString& Str) { return FString::ToFloat(Str); });
    SetPropertyHelper(TEXT("OuterRad"),    SpotLightInfo.OuterRad,    [](const FString& Str) { return FString::ToFloat(Str); });
    SetPropertyHelper(TEXT("Attenuation"), SpotLightInfo.Attenuation, [](const FString& Str) { return FString::ToFloat(Str); });
}

FVector USpotLightComponent::GetDirection()
{
    return GetWorldRotation()
           .ToQuaternion()
           .RotateVector(GetForwardVector());
}

const FSpotLightInfo& USpotLightComponent::GetSpotLightInfo() const
{
    return SpotLightInfo;
}

void USpotLightComponent::SetSpotLightInfo(const FSpotLightInfo& InSpotLightInfo)
{
    SpotLightInfo = InSpotLightInfo;
}

float USpotLightComponent::GetRadius() const
{
    return SpotLightInfo.Radius;
}

void USpotLightComponent::SetRadius(float InRadius)
{
    SpotLightInfo.Radius = InRadius;
}

FLinearColor USpotLightComponent::GetLightColor() const
{
    return SpotLightInfo.LightColor;
}

void USpotLightComponent::SetLightColor(const FLinearColor& InColor)
{
    SpotLightInfo.LightColor = InColor;
}



float USpotLightComponent::GetIntensity() const
{
    return SpotLightInfo.Intensity;
}

void USpotLightComponent::SetIntensity(float InIntensity)
{
    SpotLightInfo.Intensity = InIntensity;
}

int USpotLightComponent::GetType() const
{
    return SpotLightInfo.Type;
}

void USpotLightComponent::SetType(int InType)
{
    SpotLightInfo.Type = InType;
}

float USpotLightComponent::GetInnerRad() const
{
    return SpotLightInfo.InnerRad;
}

void USpotLightComponent::SetInnerRad(float InInnerCos)
{
    SpotLightInfo.InnerRad = InInnerCos;
}

float USpotLightComponent::GetOuterRad() const
{
    return SpotLightInfo.OuterRad;
}

void USpotLightComponent::SetOuterRad(float InOuterCos)
{
    SpotLightInfo.OuterRad = InOuterCos;
}

float USpotLightComponent::GetInnerDegree() const
{
    return SpotLightInfo.InnerRad * (180.0f / PI);
}

void USpotLightComponent::SetInnerDegree(float InInnerDegree)
{
    SpotLightInfo.InnerRad = InInnerDegree * (PI / 180.0f);
}

float USpotLightComponent::GetOuterDegree() const
{
    return SpotLightInfo.OuterRad * (180 / PI);
}

void USpotLightComponent::SetOuterDegree(float InOuterDegree)
{
    SpotLightInfo.OuterRad = InOuterDegree * (PI / 180.0f);
}
