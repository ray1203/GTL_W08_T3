#include "GameEngine.h"

#include "EditorEngine.h"
#include "World/World.h"
#include "Level.h"
#include "Actors/Cube.h"
#include "Actors/DirectionalLightActor.h"
#include "GameFramework/Actor.h"
#include "Classes/Engine/AssetManager.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Editor/LevelEditor/SLevelEditor.h"

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
}

void UGameEngine::Tick(float DeltaTime)
{
     for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::Game)
        {
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

            /*if (GEngine->bRestartGame)
            {
                GEngine->bRestartGame = false;
                RestartGame();
            }*/
            LoadScene();
        }
    }
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
}
void UGameEngine::LoadScene()
{
    //Load
    if (!bLoadScene)return;
    bLoadScene = false;
    for (auto i : ActiveWorld->GetActiveLevel()->Actors)i->Destroy();
    if (ActiveWorld->GetActiveLevel())
    {
        ActiveWorld->GetActiveLevel()->Release();
    }
    //ActiveWorld->Release();
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
