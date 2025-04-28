#include "SpotLightActor.h"
#include "Components/BillboardComponent.h"
#include "Components/Light/SpotLightComponent.h"

ASpotLight::ASpotLight()
{
    SpotLightComponent = AddComponent<USpotLightComponent>(TEXT("SpotLightComponent"));

    RootComponent = SpotLightComponent;
}
