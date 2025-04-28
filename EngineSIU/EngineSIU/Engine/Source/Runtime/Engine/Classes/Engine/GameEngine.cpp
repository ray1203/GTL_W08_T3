#include "GameEngine.h"

#include "World/World.h"
#include "Level.h"
#include "Actors/Cube.h"
#include "Actors/DirectionalLightActor.h"
#include "GameFramework/Actor.h"
#include "Classes/Engine/AssetManager.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Editor/LevelEditor/SLevelEditor.h"
#include <WindowsCursor.h>
#include <Sound/SoundManager.h>

void UGameEngine::Init()
{
    Super::Init();

    GEngine = this;

    FWorldContext& GameWorldContext = CreateNewWorldContext(EWorldType::Game);

    InitGameWorld(GameWorldContext);

    if (AssetManager == nullptr)
    {
        AssetManager = FObjectFactory::ConstructObject<UAssetManager>(this);
        assert(AssetManager);
        AssetManager->InitAssetManager();
    }
    SceneNames.Add(TEXT("Contents\\StartGameScene.scene"));
    SceneNames.Add(TEXT("Contents\\MainGameScene.scene"));
    LoadScene(0);

    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();
    if (Handler)
    {
        Handler->OnKeyDownDelegate.AddLambda([](const FKeyEvent& KeyEvent)
            {
                if (KeyEvent.GetCharacter() == VK_ESCAPE)
                {
                    FWindowsCursor::SetShowMouseCursor(!FWindowsCursor::GetShowMouseCursor());
                }
            });
    }


}

void UGameEngine::Tick(float DeltaTime)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::Game)
        {
            if (GEngine->bRestartGame)
            {
                GEngine->bRestartGame = false;
                RestartGame();
            }
            if (bLoadScene)LoadScene();
            if (UWorld* World = WorldContext->World())
            {
                World->Tick(DeltaTime);
                ULevel* Level = World->GetActiveLevel();
                if (Level)
                {
                    TArray CachedActors = Level->Actors;
                    for (AActor* Actor : CachedActors)
                    {
                        if (Actor)
                        {
                            Actor->Tick(DeltaTime);
                        }
                    }
                }
            }
        }
    }

    FSoundManager::Instance().Update();
}

void UGameEngine::InitGameWorld(FWorldContext& GameWorldContext)
{
    GameWorld = UWorld::CreateWorld(this, EWorldType::Game, FString("GameWorld"));
    GameWorldContext.SetCurrentWorld(GameWorld);
    ActiveWorld = GameWorld;
}
void UGameEngine::LoadScene(int index)
{
    bLoadScene = true;
    LoadSceneIndex = index;

    if (index == 1)
    {
        FWindowsCursor::SetShowMouseCursor(false);
        FSoundManager::Instance().PlayBGM(true);
    }
}
void UGameEngine::LoadScene()
{
    //Load
    if (!bLoadScene)return;
    bLoadScene = false;
    if (ActiveWorld->GetActiveLevel())
    {
        ActiveWorld->GetActiveLevel()->Release();
    }
    LoadWorld(SceneNames[LoadSceneIndex]);
}
void UGameEngine::RestartGame()
{
    if (!bRestartGame)return;
    /*if (ActiveWorld->GetActiveLevel())
    {
        ActiveWorld->GetActiveLevel()->Release();
    }*/
    LoadScene(1);
}
