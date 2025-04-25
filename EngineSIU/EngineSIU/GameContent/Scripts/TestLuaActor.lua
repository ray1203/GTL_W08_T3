function BeginPlay()
    print("[BeginPlay] " .. obj.UUID)
    print("TestLuaActor")
    obj:PrintLocation()
    obj.Velocity = Vector(1,1,1)
end

function EndPlay()
    print("[EndPlay] " .. obj.UUID)
    obj:PrintLocation()
end

function OnOverlap(OtherActor)
    OtherActor:PrintLocation();
end

function Tick(dt)
    obj.Location = obj.Location + obj.Velocity * dt
    obj:PrintLocation()
end