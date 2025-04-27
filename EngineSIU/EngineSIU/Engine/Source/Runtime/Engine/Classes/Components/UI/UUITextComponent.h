#pragma once
#include "UUIComponent.h"


class UUITextComponent : public UUIComponent
{
    DECLARE_CLASS(UUITextComponent, UUIComponent)
public:
    UUITextComponent()=default;
    void SetText(const std::string& InText) { Text = InText; }
    const std::string& GetText() const { return Text; }

    virtual void TickComponent(float DeltaTime) override;

    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& Properties) override;


    virtual void RenderUI() override;
    FLinearColor TextColor = FLinearColor::White;
    float TextScale = 1.0f;
private:
    std::string Text = "DefaultText";
};
