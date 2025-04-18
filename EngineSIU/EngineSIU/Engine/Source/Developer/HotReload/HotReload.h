#pragma once


class FHotReload
{
    FHotReload() = default;

public:
    static FHotReload& Get()
    {
        static FHotReload Instance;
        return Instance;
    }
};
