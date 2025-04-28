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

    void SetButtonSize(const FVector2D& InSize) { ButtonSize = InSize; }
    void SetFontScale(float InScale) { FontScale = InScale; }
    FVector2D GetButtonSize() const { return ButtonSize; }
    float GetFontScale() const { return FontScale; }

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

    FVector2D ButtonSize = FVector2D(300, 60); // 기본 버튼 크기
    float FontScale = 1.0f; // 기본 폰트 배율
};
