#pragma once

#include <string>

#include "Components/ActorComponent.h"

class UUIComponent : public UActorComponent
{
    DECLARE_CLASS(UUIComponent, UActorComponent)
public:
    UUIComponent()
    {
        bNoMove = true;
        bNoResize = true;
        bNoTitleBar = true;
        bNoSavedSettings = true;
        bNoBackground = false;
        bNoInputs = false;
        bAutoSize = false;
    }
    // 적용될 UI 속성
    bool bNoMove;
    bool bNoResize;
    bool bNoTitleBar;
    bool bNoSavedSettings;
    bool bNoBackground;

    // 추가 속성
    bool bNoInputs;
    bool bAutoSize;

    int32 ZOrder = 0; // 미래 확장용


    virtual void InitializeComponent() override;
    FVector2D Position = FVector2D(100.0f, 100.0f);
    FVector2D Size = FVector2D(200.0f, 100.0f);
    virtual void TickComponent(float DeltaTime) override;
    virtual void RenderUI();

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& Properties) override;

    // lua에서 식별할 수 있게 이름 설정
    void SetWidgetName(const std::string& InName) { WidgetName = InName; }
    const std::string& GetWidgetName() const { return WidgetName; }
protected:
    bool BeginWidget();  // 내부에서 SetNextWindowPos/Size + Begin
    void EndWidget();    // End 호출
protected:
    std::string WidgetName;
    uint8 bVisible : 1 = true;
    static inline int32 GlobalWidgetCounter = 0;
};
