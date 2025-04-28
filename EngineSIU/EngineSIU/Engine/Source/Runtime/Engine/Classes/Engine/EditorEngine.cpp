#include "EditorEngine.h"

#include "World/World.h"
#include "Level.h"
#include "Actors/Cube.h"
#include "Actors/DirectionalLightActor.h"
#include "GameFramework/Actor.h"
#include "Classes/Engine/AssetManager.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Editor/LevelEditor/SLevelEditor.h"
#include <Camera/CameraComponent.h>
#include "Components/Shapes/SphereComponent.h"
#include "Components/ProjectileMovementComponent.h"
#include "Physics/PhysicsScene.h"
#include <Actors/Player.h>
#include <WindowsCursor.h>

namespace PrivateEditorSelection
{
    static AActor* GActorSelected = nullptr;
    static AActor* GActorHovered = nullptr;

    static UActorComponent* GComponentSelected = nullptr;
    static USceneComponent* GComponentHovered = nullptr;
}

void UEditorEngine::Init()
{
    Super::Init();

    // Initialize the engine
    GEngine = this;

    FWorldContext& EditorWorldContext = CreateNewWorldContext(EWorldType::Editor);

    InitEditorWorld(EditorWorldContext);


    if (AssetManager == nullptr)
    {
        AssetManager = FObjectFactory::ConstructObject<UAssetManager>(this);
        assert(AssetManager);
        AssetManager->InitAssetManager();
    }

#ifdef _DEBUG
    //AActor* Actor = EditorWorld->SpawnActor<ACube>();
    //UCameraComponent* CameraComp = Actor->AddComponent<UCameraComponent>(TEXT("Camera"));
    //CameraComp->SetupAttachment(Actor->GetRootComponent());
    
    APlayer* Player = EditorWorld->SpawnActor<APlayer>();

    ADirectionalLight* DirLight = EditorWorld->SpawnActor<ADirectionalLight>();
    DirLight->SetActorRotation(FRotator(20, -61, 11));
    DirLight->SetActorLocation(FVector(0, 0, 20));
    DirLight->SetIntensity(2.f);
#endif
}

void UEditorEngine::Tick(float DeltaTime)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::Editor)
        {
            if (UWorld* World = WorldContext->World())
            {
                // TODO: World에서 EditorPlayer 제거 후 Tick 호출 제거 필요.
                World->Tick(DeltaTime);
                EditorPlayer->Tick(DeltaTime);
                ULevel* Level = World->GetActiveLevel();
                TArray CachedActors = Level->Actors;
                if (Level)
                {
                    for (AActor* Actor : CachedActors)
                    {
                        if (Actor && Actor->IsActorTickInEditor())
                        {
                            Actor->Tick(DeltaTime);
                        }
                    }
                }
            }
        }
        else if (WorldContext->WorldType == EWorldType::PIE)
        {
            if (UWorld* World = WorldContext->World())
            {
                World->Tick(DeltaTime);
                ULevel* Level = World->GetActiveLevel();
                TArray CachedActors = Level->Actors;
                if (Level)
                {
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
}

void UEditorEngine::StartPIE()
{
    if (PIEWorld)
    {
        UE_LOG(ELogLevel::Warning, TEXT("PIEWorld already exists!"));
        return;
    }

    GEngineLoop.GetLevelEditor()->UnbindEditorInput();

    // Bind F3 Key to Show cursor
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();
    if (Handler)
    {
        ShowCursorDelegateHandle = Handler->OnKeyDownDelegate.AddLambda([](const FKeyEvent& KeyEvent)
            {
                if (KeyEvent.GetCharacter() == 'T')
                {
                    FWindowsCursor::SetShowMouseCursor(!FWindowsCursor::GetShowMouseCursor());
                }
            });
    }
    FWorldContext& PIEWorldContext = CreateNewWorldContext(EWorldType::PIE);

    PIEWorld = Cast<UWorld>(EditorWorld->Duplicate(this));
    PIEWorld->WorldType = EWorldType::PIE;

    // PIE의 경우에는 PhysicsScene을 여기서 생성
    // 게임의 경우에는 GameEngine에서 생성해야할듯...


    PIEWorldContext.SetCurrentWorld(PIEWorld);
    ActiveWorld = PIEWorld;
    
    PIEWorld->BeginPlay();
    // 여기서 Actor들의 BeginPlay를 해줄지 안에서 해줄 지 고민.
    //WorldList.Add(GetWorldContextFromWorld(PIEWorld)); // 이미 CreateNewWorldContext에서 하고있음

    FWindowsCursor::SetShowMouseCursor(false);
}

void UEditorEngine::EndPIE()
{
    if (PIEWorld)
    {
        //WorldList.Remove(*GetWorldContextFromWorld(PIEWorld.get()));
        WorldList.Remove(GetWorldContextFromWorld(PIEWorld));
        PIEWorld->Release();
        GUObjectArray.MarkRemoveObject(PIEWorld);
        PIEWorld = nullptr;

        // TODO: PIE에서 EditorWorld로 돌아올 때, 기존 선택된 Picking이 유지되어야 함. 현재는 에러를 막기위해 임시조치.
        SelectActor(nullptr);
        SelectComponent(nullptr);
    }
    // 다시 EditorWorld로 돌아옴.
    ActiveWorld = EditorWorld;
    FWindowsCursor::SetShowMouseCursor(true);

    // 이벤트 해제
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();
    if (Handler && ShowCursorDelegateHandle.has_value())
    {
        Handler->OnKeyDownDelegate.Remove(ShowCursorDelegateHandle.value());
    }
}

FWorldContext& UEditorEngine::GetEditorWorldContext(/*bool bEnsureIsGWorld*/)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::Editor)
        {
            return *WorldContext;
        }
    }
    return CreateNewWorldContext(EWorldType::Editor);
}

FWorldContext* UEditorEngine::GetPIEWorldContext(/*int32 WorldPIEInstance*/)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::PIE)
        {
            return WorldContext;
        }
    }
    return nullptr;
}

void UEditorEngine::SelectActor(AActor* InActor) const
{
    if (InActor && CanSelectActor(InActor))
    {
        PrivateEditorSelection::GActorSelected = InActor;
    }
    else if (InActor == nullptr)
    {
        PrivateEditorSelection::GActorSelected = nullptr;
    }
}

bool UEditorEngine::CanSelectActor(const AActor* InActor) const
{
    return InActor != nullptr && InActor->GetWorld() == ActiveWorld && !InActor->IsActorBeingDestroyed();
}

AActor* UEditorEngine::GetSelectedActor() const
{
    return PrivateEditorSelection::GActorSelected;
}

void UEditorEngine::HoverActor(AActor* InActor) const
{
    if (InActor)
    {
        PrivateEditorSelection::GActorHovered = InActor;
    }
    else if (InActor == nullptr)
    {
        PrivateEditorSelection::GActorHovered = nullptr;
    }
}

void UEditorEngine::NewWorld()
{
    SelectActor(nullptr);
    SelectComponent(nullptr);

    if (ActiveWorld->GetActiveLevel())
    {
        ActiveWorld->GetActiveLevel()->Release();
    }
}

void UEditorEngine::SelectComponent(UActorComponent* InComponent) const
{
    if (InComponent && CanSelectComponent(InComponent))
    {
        PrivateEditorSelection::GComponentSelected = InComponent;
    }
    else if (InComponent == nullptr)
    {
        PrivateEditorSelection::GComponentSelected = nullptr;
    }
}

bool UEditorEngine::CanSelectComponent(const UActorComponent* InComponent) const
{
    return InComponent != nullptr && InComponent->GetOwner() && InComponent->GetOwner()->GetWorld() == ActiveWorld && !InComponent->GetOwner()->IsActorBeingDestroyed();
}

UActorComponent* UEditorEngine::GetSelectedComponent() const
{
    return PrivateEditorSelection::GComponentSelected;
}

void UEditorEngine::HoverComponent(USceneComponent* InComponent) const
{
    if (InComponent)
    {
        PrivateEditorSelection::GComponentHovered = InComponent;
    }
    else if (InComponent == nullptr)
    {
        PrivateEditorSelection::GComponentHovered = nullptr;
    }
}

AEditorPlayer* UEditorEngine::GetEditorPlayer() const
{
    return EditorPlayer;
}

void UEditorEngine::InitEditorWorld(FWorldContext& EditorWorldContext)
{
    if (bEditorWorldInit)
    {
        return;
    }
    EditorWorld = UWorld::CreateWorld(this, EWorldType::Editor, FString("EditorWorld"));

    EditorWorldContext.SetCurrentWorld(EditorWorld);
    ActiveWorld = EditorWorld;
    EditorPlayer = FObjectFactory::ConstructObject<AEditorPlayer>(this);
    bEditorWorldInit = true;
}
