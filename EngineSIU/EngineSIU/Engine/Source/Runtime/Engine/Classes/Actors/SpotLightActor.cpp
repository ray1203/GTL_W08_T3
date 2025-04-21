#include "SpotLightActor.h"
#include "Components/BillboardComponent.h"
#include "Components/Light/SpotLightComponent.h"

ASpotLight::ASpotLight()
{
    SpotLightComponent = AddComponent<USpotLightComponent>(TEXT("SpotLightComponent"));
    BillboardComponent = AddComponent<UBillboardComponent>(TEXT("BillboardComponent"));

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/SpotLight_64x.png");
    BillboardComponent->bIsEditorBillboard = true;

    SpotLightComponent->AttachToComponent(RootComponent);
}
