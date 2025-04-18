#pragma once
#include "RuntimeInclude.h"

struct ImGuiContext;
struct IRCCppMainLoop;
RUNTIME_MODIFIABLE_INCLUDE;


struct SystemTable
{
    IRCCppMainLoop* MainLoop = nullptr;
    ImGuiContext* ImGuiContext = nullptr;
};
