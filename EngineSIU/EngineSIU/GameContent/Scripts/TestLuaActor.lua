function BeginPlay()
    print("[Lua] BeginPlay for " .. obj:GetActorLabel())
    obj:SetActorLocation(Vector(300, 100, 0))
end

function Tick(dt)
    local loc = obj:GetActorLocation()
    loc.x = loc.x + dt * 50.0
    obj:SetActorLocation(loc)
    print(string.format("[Lua] Tick: %.2f, Location = (%.1f, %.1f, %.1f)", dt, loc.x, loc.y, loc.z))
end

function EndPlay()
    print("[Lua] EndPlay for " .. obj:GetActorLabel())
end

function OnOverlap(other)
    print("[Lua] OnOverlap with " .. other:GetActorLabel())
end
