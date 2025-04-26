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
    virtual void RenderUI() override;

private:
    std::string Text = "DefaultText";
};
