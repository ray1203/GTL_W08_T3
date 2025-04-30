#include "GraphEditorPanel.h"
#include "ImGUI/imgui.h"
#include "Widgets/ImGraphWidget.h"
GraphEditorPanel::GraphEditorPanel()
{
    CurvePoints[0] = ImVec2(0, 0);
    CurvePoints[1] = ImVec2(1, 1);
    CurvePoints[2].x = ImGui::CurveTerminator;
    
}
void GraphEditorPanel::Render()
{
    float PanelWidth = Width * 0.4f;
    float PanelHeight = Height * 0.4f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowPos(ImVec2(Width * 0.3f, Height * 0.3f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Camera Shake Curve Editor", nullptr, PanelFlags);


    // 곡선 편집 UI
    if (ImGui::Curve("ShakeCurve", ImVec2(PanelWidth - 20, PanelHeight - 80), MaxPoints, CurvePoints, &SelectedPoint))
    {
        ImGui::Text("Shake curve modified!");
    }

    ImGui::End();
}

void GraphEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = static_cast<float>(clientRect.right - clientRect.left);
    Height = static_cast<float>(clientRect.bottom - clientRect.top);
}
