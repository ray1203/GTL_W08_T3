#include "PointLightComponent.h"

#include "UObject/Casts.h"

UPointLightComponent::UPointLightComponent()
{
    PointLightInfo.Position = GetWorldLocation();
    PointLightInfo.Radius = 30.f;

    PointLightInfo.LightColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    PointLightInfo.Intensity = 1000.f;
    PointLightInfo.Type = ELightType::POINT_LIGHT;
    PointLightInfo.Attenuation = 20.0f;
}

UObject* UPointLightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->PointLightInfo = PointLightInfo;
    }
    return NewComponent;
}

void UPointLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Radius"), FString::SanitizeFloat(PointLightInfo.Radius));
    OutProperties.Add(TEXT("LightColor"), PointLightInfo.LightColor.ToString());
    OutProperties.Add(TEXT("Intensity"), FString::SanitizeFloat(PointLightInfo.Intensity));
    OutProperties.Add(TEXT("Type"), FString::FromInt(PointLightInfo.Type));
    OutProperties.Add(TEXT("Attenuation"), FString::SanitizeFloat(PointLightInfo.Attenuation));
    OutProperties.Add(TEXT("Position"), PointLightInfo.Position.ToString());
}

void UPointLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    auto SetPropertyHelper = [&InProperties] <typename T, typename Fn>(const FString& Key, T& MemberVariable, const Fn& ConversionFunc)
    {
        if (const FString* TempStr = InProperties.Find(Key))
        {
            MemberVariable = ConversionFunc(*TempStr);
        }
    };

    SetPropertyHelper(TEXT("Radius"),      PointLightInfo.Radius,      [](const FString& Str) { return FString::ToFloat(Str); });
    SetPropertyHelper(TEXT("LightColor"),  PointLightInfo.LightColor,  [](const FString& Str) { FLinearColor Color; Color.InitFromString(Str); return Color; });
    SetPropertyHelper(TEXT("Intensity"),   PointLightInfo.Intensity,   [](const FString& Str) { return FString::ToFloat(Str); });
    SetPropertyHelper(TEXT("Type"),        PointLightInfo.Type,        [](const FString& Str) { return FString::ToInt(Str); });
    SetPropertyHelper(TEXT("Attenuation"), PointLightInfo.Attenuation, [](const FString& Str) { return FString::ToFloat(Str); });
    SetPropertyHelper(TEXT("Position"),    PointLightInfo.Position,    [](const FString& Str) { FVector Pos; Pos.InitFromString(Str); return Pos; });
}

const FPointLightInfo& UPointLightComponent::GetPointLightInfo() const
{
    return PointLightInfo;
}

void UPointLightComponent::SetPointLightInfo(const FPointLightInfo& InPointLightInfo)
{
    PointLightInfo = InPointLightInfo;
}


float UPointLightComponent::GetRadius() const
{
    return PointLightInfo.Radius;
}

void UPointLightComponent::SetRadius(float InRadius)
{
    PointLightInfo.Radius = InRadius;
}

FLinearColor UPointLightComponent::GetLightColor() const
{
    return PointLightInfo.LightColor;
}

void UPointLightComponent::SetLightColor(const FLinearColor& InColor)
{
    PointLightInfo.LightColor = InColor;
}


float UPointLightComponent::GetIntensity() const
{
    return PointLightInfo.Intensity;
}

void UPointLightComponent::SetIntensity(float InIntensity)
{
    PointLightInfo.Intensity = InIntensity;
}

int UPointLightComponent::GetType() const
{
    return PointLightInfo.Type;
}

void UPointLightComponent::SetType(int InType)
{
    PointLightInfo.Type = InType;
}
