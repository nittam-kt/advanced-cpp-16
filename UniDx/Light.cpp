#include "Light.h"

#include "LightManager.h"
#include "Debug.h"


namespace UniDx{

void Light::OnEnable()
{
    LightManager::getInstance()->registerLight(this);
}


void Light::OnDisable()
{
    LightManager::getInstance()->unregisterLight(this);
}

}
