Velocity = Vector(20,2,0)
function BeginPlay()
    print("[BeginPlay] " .. obj.UUID)
    print("TestLuaActor")
    obj:PrintLocation()
    -- obj.Velocity = Vector(2,2,0)
end

function EndPlay()
    print("[EndPlay] " .. obj.UUID)
    obj:PrintLocation()
end

function OnOverlap(OtherActor)
    OtherActor:PrintLocation();
end

function Tick(dt)
    --obj.Location = obj.Location + obj.Velocity * dt
    obj.Location = obj.Location + Velocity * dt
    obj:PrintLocation()
    obj:PrintVector(Velocity)
end