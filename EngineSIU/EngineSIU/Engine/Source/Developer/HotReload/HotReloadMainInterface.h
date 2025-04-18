#pragma once

struct IRCCppMainLoop
{
    virtual void MainLoop() = 0;
    virtual ~IRCCppMainLoop() = default;
};
