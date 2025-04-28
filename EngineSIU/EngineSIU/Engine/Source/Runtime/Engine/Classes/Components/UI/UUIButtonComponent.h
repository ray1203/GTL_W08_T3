#pragma once

#include <functional>
#include <sol/function.hpp>

#include "UUIComponent.h"

class UUIButtonComponent : public UUIComponent
{
    DECLARE_CLASS(UUIButtonComponent, UUIComponent)
public:
    UUIButtonComponent() = default;
    void SetLabel(const std::string& InLabel) { Label = InLabel; }
    const std::string& GetLabel() const { return Label; }

    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& Properties) override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void RenderUI() override;
    void SetOnClick(const std::function<void()>& Callback) { NativeCallback = Callback; }
    void BindLuaCallback(sol::function InFunc) { LuaCallback = InFunc; }

    void OnClick();

private:
    std::string Label = "Click Me";
    std::function<void()> NativeCallback = nullptr;
    sol::function LuaCallback;
};
