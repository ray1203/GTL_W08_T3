function BeginPlay()
    print("[BeginPlay] " .. obj.UUID)
    PlaySFX("MonkeySpawn")
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
    PlaySFX("Throw")

end

local shoot_timer = 0
local angry_timer = 0
local timing = 2.0

function Tick(dt)
    shoot_timer = shoot_timer + dt
    angry_timer = angry_timer + dt

    if angry_timer >= 20.0 then
        timing = 1.0
        angry_timer = 0
    else
        timing = 2.0
    end

    if shoot_timer >= timing then
        ShootSSal()
        shoot_timer = 0  -- 또는 shoot_timer = 0
    end
end
