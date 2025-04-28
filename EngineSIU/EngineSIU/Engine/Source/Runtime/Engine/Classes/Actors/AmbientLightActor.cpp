#include "AmbientLightActor.h"
#include "Components/Light/AmbientLightComponent.h"
#include "Components/BillboardComponent.h"

AAmbientLight::AAmbientLight()
{
    AmbientLightComponent = AddComponent<UAmbientLightComponent>(TEXT("AmbientLightComponent"));

    RootComponent = AmbientLightComponent;


}

AAmbientLight::~AAmbientLight()
{
}
