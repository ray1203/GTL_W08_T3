#include "DirectionalLightActor.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Components/BillboardComponent.h"

ADirectionalLight::ADirectionalLight()
{
    DirectionalLightComponent = AddComponent<UDirectionalLightComponent>(TEXT("DirectionalLightComponent"));
    BillboardComponent = AddComponent<UBillboardComponent>(TEXT("BillboardComponent"));

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/DirectionalLight_64x.png");
    BillboardComponent->bIsEditorBillboard = true;

    DirectionalLightComponent->AttachToComponent(RootComponent);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ADirectionalLight::SetIntensity(float Intensity)
{
    if (DirectionalLightComponent)
    {
        DirectionalLightComponent->SetIntensity(Intensity);
    }
}
