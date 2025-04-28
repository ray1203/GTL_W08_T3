#include "PointLightActor.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/BillboardComponent.h"

APointLight::APointLight()
{
    PointLightComponent = AddComponent<UPointLightComponent>(TEXT("PointLightComponent"));
    RootComponent = PointLightComponent;
}
