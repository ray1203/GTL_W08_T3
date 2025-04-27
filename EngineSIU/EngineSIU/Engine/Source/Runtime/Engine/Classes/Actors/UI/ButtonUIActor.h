#pragma once
#include "GameFramework/Actor.h"

class UUIButtonComponent;

class AButtonUIActor : public AActor
{
    DECLARE_CLASS(AButtonUIActor, AActor)

public:
    AButtonUIActor();

    void SetLabel(const std::string& InLabel);
    void SetOnClick(std::function<void()> Callback);

protected:
    UPROPERTY(UUIButtonComponent*, ButtonComponent, = nullptr);
};
