#include "AmbientLightComponent.h"

#include "UObject/Casts.h"

UAmbientLightComponent::UAmbientLightComponent()
{
    AmbientLightInfo.AmbientColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
    //AmbientLightInfo.AmbientColor = FLinearColor(1.f, 1.f, 1.f, 1.f);
}

UObject* UAmbientLightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->AmbientLightInfo = AmbientLightInfo;
    return NewComponent;
}

void UAmbientLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("AmbientColor"), AmbientLightInfo.AmbientColor.ToString());
}

void UAmbientLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    auto SetPropertyHelper = [&InProperties] <typename T, typename Fn>(const FString& Key, T& MemberVariable, const Fn& ConversionFunc)
    {
        if (const FString* TempStr = InProperties.Find(Key))
        {
            MemberVariable = ConversionFunc(*TempStr);
        }
    };

    SetPropertyHelper(TEXT("AmbientColor"), AmbientLightInfo.AmbientColor, [](const FString& Str) { FLinearColor Color; Color.InitFromString(Str); return Color; });
}

const FAmbientLightInfo& UAmbientLightComponent::GetAmbientLightInfo() const
{
    return AmbientLightInfo;
}

void UAmbientLightComponent::SetAmbientLightInfo(const FAmbientLightInfo& InAmbient)
{
    AmbientLightInfo = InAmbient;
}

FLinearColor UAmbientLightComponent::GetLightColor() const
{
    return AmbientLightInfo.AmbientColor;
}

void UAmbientLightComponent::SetLightColor(const FLinearColor& InColor)
{
    AmbientLightInfo.AmbientColor = InColor;
}
