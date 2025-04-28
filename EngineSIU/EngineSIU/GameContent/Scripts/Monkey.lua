function BeginPlay()
    print("[BeginPlay] " .. obj.UUID)
    obj:PrintLocation()
end

function EndPlay()
    print("[EndPlay] " .. obj.UUID)
    obj:PrintLocation()
end

function OnOverlap(OtherActor)
    OtherActor:PrintLocation()
end

function ShootSSal()
    local shootPos = obj.Location + obj.RightVector * 3;
    local playerPos = obj.PlayerPosition

    obj:ShootProjectile(shootPos, playerPos, 100)
end

local shoot_timer = 0

function Tick(dt)
    shoot_timer = shoot_timer + dt
    if shoot_timer >= 2.0 then
        ShootSSal()
        shoot_timer = shoot_timer - 2.0  -- 또는 shoot_timer = 0
    end
end