function BeginPlay()
    print("[BeginPlay] " .. obj.UUID)
    obj:PrintLocation()
end

function EndPlay()
    print("[EndPlay] " .. obj.UUID)
    obj:PrintLocation()
end

function OnOverlap(OtherActor)
    OtherActor:PrintLocation();
end

function Tick(dt)
    -- obj.Location = obj.Location + Vector(-0.1,0,0)
    -- obj.Location = obj.Location + obj.Velocity * dt
    -- obj:PrintLocation()
end
