#pragma once

#include <functional>

#include "UUIComponent.h"

class UUIButtonComponent : public UUIComponent
{
    DECLARE_CLASS(UUIButtonComponent, UUIComponent)
public:
    UUIButtonComponent() = default;
    void SetLabel(const std::string& InLabel) { Label = InLabel; }
    const std::string& GetLabel() const { return Label; }

    void SetOnClick(const std::function<void()>& Callback) { OnClick = Callback; }
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& Properties) override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void RenderUI() override;

    std::function<void()> OnClick = nullptr;

private:
    std::string Label = "Click Me";
};
