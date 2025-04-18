#include "IObject.h"
#include "ObjectInterfacePerModule.h"
#include "Developer/HotReload/HotReload.h"
#include "Developer/HotReload/HotReloadMainInterface.h"


RUNTIME_MODIFIABLE_INCLUDE;


// RCC++ uses interface Id's to distinguish between different classes
// here we have only one, so we don't need a header for this enum and put it in the same
// source code file as the rest of the code
enum InterfaceIDEnumConsoleExample : uint8_t
{
    IID_IRCCPP_MAIN_LOOP = IID_ENDInterfaceID, // IID_ENDInterfaceID from IObject.h InterfaceIDEnum

    IID_ENDInterfaceIDEnumConsoleExample
};

struct RCCppMainLoop
    : public IRCCppMainLoop
    , public TInterface<IID_IRCCPP_MAIN_LOOP, IObject>
{
    RCCppMainLoop();
    virtual void MainLoop() override;
};

RCCppMainLoop::RCCppMainLoop()
{
    SystemTable* Table = PerModuleInterface::GetInstance()->GetSystemTable();
    Table->MainLoop = this;
}

void RCCppMainLoop::MainLoop()
{
}

REGISTERSINGLETON(RCCppMainLoop, true);
