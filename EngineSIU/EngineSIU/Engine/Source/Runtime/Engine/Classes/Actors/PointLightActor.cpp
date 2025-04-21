#include "PointLightActor.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/BillboardComponent.h"

APointLight::APointLight()
{
    PointLightComponent = AddComponent<UPointLightComponent>(TEXT("PointLightComponent"));
    BillboardComponent = AddComponent<UBillboardComponent>(TEXT("BillboardComponent"));

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/PointLight_64x.png");
    BillboardComponent->bIsEditorBillboard = true;

    PointLightComponent->AttachToComponent(RootComponent);
}
