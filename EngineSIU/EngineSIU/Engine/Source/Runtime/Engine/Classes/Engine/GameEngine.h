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
    void LoadScene(int index);
    bool bLoadScene = false;
private:
    void InitGameWorld(FWorldContext& GameWorldContext);
    void LoadScene();
    virtual void RestartGame() override;
    TArray<FString> SceneNames;
    int LoadSceneIndex;
};
