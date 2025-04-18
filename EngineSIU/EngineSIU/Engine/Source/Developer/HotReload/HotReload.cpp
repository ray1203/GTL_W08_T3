#include "HotReload.h"
#include "RuntimeObjectSystem.h"

IRuntimeObjectSystem* FHotReload::RuntimeObjectSystemInstance = nullptr;
FHotReloadLogger FHotReload::RCLogger = FHotReloadLogger();


bool FHotReload::Initialize()
{
    // RCC++ Initialization
    RuntimeObjectSystemInstance = new RuntimeObjectSystem;
    if (!RuntimeObjectSystemInstance->Initialise(&RCLogger, nullptr))
    {
        delete RuntimeObjectSystemInstance;
        RuntimeObjectSystemInstance = nullptr;
        return false;
    }
    return true;
}

void FHotReload::Shutdown()
{
    delete RuntimeObjectSystemInstance;
    RuntimeObjectSystemInstance = nullptr;
}
