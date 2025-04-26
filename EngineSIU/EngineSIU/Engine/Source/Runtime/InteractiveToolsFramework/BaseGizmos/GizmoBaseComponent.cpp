#include "GizmoBaseComponent.h"

#include "TransformGizmo.h"
#include "GameFramework/Actor.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealClient.h"

int UGizmoBaseComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    int nIntersections = 0;
    if (staticMesh == nullptr) return 0;
    OBJ::FStaticMeshRenderData* renderData = staticMesh->GetRenderData();
    FStaticMeshVertex* vertices = renderData->Vertices.GetData();
    int vCount = renderData->Vertices.Num();
    UINT* indices = renderData->Indices.GetData();
    int iCount = renderData->Indices.Num();

    if (!vertices) return 0;
    BYTE* pbPositions = reinterpret_cast<BYTE*>(renderData->Vertices.GetData());

    int nPrimitives = (!indices) ? (vCount / 3) : (iCount / 3);
    float fNearHitDistance = FLT_MAX;
    for (int i = 0; i < nPrimitives; i++) {
        int idx0, idx1, idx2;
        if (!indices) {
            idx0 = i * 3;
            idx1 = i * 3 + 1;
            idx2 = i * 3 + 2;
        }
        else {
            idx0 = indices[i * 3];
            idx2 = indices[i * 3 + 1];
            idx1 = indices[i * 3 + 2];
        }

        // 각 삼각형의 버텍스 위치를 FVector로 불러옵니다.
        uint32 stride = sizeof(FStaticMeshVertex);
        FVector v0 = *reinterpret_cast<FVector*>(pbPositions + idx0 * stride);
        FVector v1 = *reinterpret_cast<FVector*>(pbPositions + idx1 * stride);
        FVector v2 = *reinterpret_cast<FVector*>(pbPositions + idx2 * stride);

        float fHitDistance;
        if (IntersectRayTriangle(rayOrigin, rayDirection, v0, v1, v2, fHitDistance)) {
            if (fHitDistance < fNearHitDistance) {
                pfNearHitDistance = fNearHitDistance = fHitDistance;
            }
            nIntersections++;
        }

    }
    return nIntersections;
}

void UGizmoBaseComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    if (!GetOwner())
    {
        return;
    }
    
    if (FEditorViewportClient* ViewportClient = Cast<ATransformGizmo>(GetOwner())->GetAttachedViewport())
    {
        if (ViewportClient->IsPerspective())
        {
            // 월드 → 카메라 공간 변환 (ProjectionMatrix는 빼야 함)
            FMatrix ViewMatrix = ViewportClient->GetViewMatrix();
            FVector CameraSpaceLocation = ViewMatrix.TransformPosition(GetOwner()->GetActorLocation());

            float Distance = FMath::Abs(CameraSpaceLocation.Z); // 카메라와의 실제 거리
            float ScreenHeight = ViewportClient->GetViewportResource()->GetD3DViewport().Height;
            float FOV = ViewportClient->ViewFOV * (PI / 180.0f);

            float DesiredScreenHeight = 128.f; // 원하는 화면상 크기(픽셀)
            float DrawScale = 2.0f * Distance * FMath::Tan(FOV * 0.5f) * (DesiredScreenHeight / ScreenHeight);

            DrawScale *= GizmoScale;
            RelativeScale3D = FVector(DrawScale);
        }
        else
        {
            float Scaler = FEditorViewportClient::OrthoSize * GizmoScale;
            RelativeScale3D = FVector(Scaler);
        }
    }
}
