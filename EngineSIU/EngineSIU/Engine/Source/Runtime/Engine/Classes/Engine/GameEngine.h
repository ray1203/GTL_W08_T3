#pragma once
#include "Engine.h"

class UGameEngine : public UEngine
{
    DECLARE_CLASS(UGameEngine, UEngine)
public:
    UGameEngine() = default;

    virtual void Init() override;
    virtual void Tick(float DeltaTime) override;

    UWorld* GameWorld = nullptr;

private:
    void InitGameWorld(FWorldContext& GameWorldContext);
    void LoadInitialScene() const;
    virtual void RestartGame() const override;
};
