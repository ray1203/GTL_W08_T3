#pragma once

#include <string>

#include "Components/ActorComponent.h"

class UUIComponent : public UActorComponent
{
    DECLARE_CLASS(UUIComponent, UActorComponent)
public:
    UUIComponent() = default;
    virtual void InitializeComponent() override;
    FVector2D Position = FVector2D(100.0f, 100.0f);
    FVector2D Size = FVector2D(200.0f, 100.0f);
    virtual void TickComponent(float DeltaTime) override;
    virtual void RenderUI();

    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& Properties) override;

    // lua에서 식별할 수 있게 이름 설정
    void SetWidgetName(const std::string& InName) { WidgetName = InName; }
    const std::string& GetWidgetName() const { return WidgetName; }
protected:
    bool BeginWidget();  // 내부에서 SetNextWindowPos/Size + Begin
    void EndWidget();    // End 호출
protected:
    std::string WidgetName = "UnnamedUI";
    uint8 bVisible : 1 = true;
    static inline int32 GlobalWidgetCounter = 0;
};
