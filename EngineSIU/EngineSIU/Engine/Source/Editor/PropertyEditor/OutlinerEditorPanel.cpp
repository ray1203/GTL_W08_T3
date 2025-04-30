#include "OutlinerEditorPanel.h"
#include "World/World.h"
#include "GameFramework/Actor.h"
#include "Engine/EditorEngine.h"
#include <functional>

#include "Components/UI/UUIComponent.h"
#include "ImGUI/imgui.h"


void OutlinerEditorPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();
    
    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.3f;

    float PanelPosX = (Width) * 0.8f + 5.0f;
    float PanelPosY = 5.0f;

    ImVec2 MinSize(140, 100);
    ImVec2 MaxSize(FLT_MAX, 500);
    
    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    
    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    
    /* Render Start */
    ImGui::Begin("Outliner", nullptr, PanelFlags);

    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine) return;

    ULevel* Level = Engine->ActiveWorld->GetActiveLevel();
    if (!Level) return;

    float ContentRegionWidth = ImGui::GetContentRegionAvail().x;
    float HalfHeight = PanelHeight * 0.45f;

    // === SceneComponent 트리 영역 ===
    ImGui::Text("Scene Components");
    ImGui::Separator();
    ImGui::BeginChild("SceneTree", ImVec2(ContentRegionWidth, HalfHeight), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

    std::function<void(USceneComponent*)> CreateNode = [&CreateNode, &Engine](USceneComponent* InComp)
        {
            FString Name = (InComp == InComp->GetOwner()->GetRootComponent())
                ? InComp->GetOwner()->GetActorLabel()
                : InComp->GetName();

            ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow;
            if (InComp->GetAttachChildren().IsEmpty())
                Flags |= ImGuiTreeNodeFlags_Leaf;

            bool bOpen = ImGui::TreeNodeEx(*Name, Flags);

            if (ImGui::IsItemClicked())
            {
                Engine->SelectActor(InComp->GetOwner());
                Engine->SelectComponent(InComp);
            }

            if (bOpen)
            {
                for (USceneComponent* Child : InComp->GetAttachChildren())
                {
                    CreateNode(Child);
                }
                ImGui::TreePop();
            }
        };

    for (AActor* Actor : Level->Actors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            CreateNode(Actor->GetRootComponent());
        }
    }

    ImGui::EndChild();

    // === UIComponent 트리 영역 ===
    ImGui::Spacing();
    ImGui::Text("UI Components");
    ImGui::Separator();
    ImGui::BeginChild("UIComponentList", ImVec2(ContentRegionWidth, HalfHeight), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

    for (AActor* Actor : Level->Actors)
    {
        if (!Actor) continue;

        const TSet<UActorComponent*>& Components = Actor->GetComponents();
        for (UActorComponent* Comp : Components)
        {
            if (UUIComponent* UIComp = Cast<UUIComponent>(Comp))
            {
                FString Label = FString::Printf(TEXT("%s [%s]"), *Actor->GetActorLabel(), *UIComp->GetClass()->GetName());

                if (ImGui::Selectable(*Label))
                {
                    Engine->SelectActor(Actor);
                    Engine->SelectComponent(UIComp);
                }
            }
        }
    }

    ImGui::EndChild();
    ImGui::End();
}
    
void OutlinerEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
