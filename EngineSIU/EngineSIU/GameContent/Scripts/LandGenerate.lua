function BeginPlay()
end

function EndPlay()
end

function OnGenerated(LastLocation)
    local far = 13
    local near = 7
    -- dx, dy�� -3~3 ������ �����ϰ� ����: [-6,-3) �Ǵ� (3,6]
    local function rand_exclude(min1, max1, min2, max2)
        if math.random() < 0.5 then
            return math.random() * (max1 - min1) + min1
        else
            return math.random() * (max2 - min2) + min2
        end
    end

    local dx = rand_exclude(-far, -near, near, far)
    local dy = rand_exclude(-far, -near, near, far)
    local dz = math.random(2, 5) -- z�� �׻� ����(3~8)

    obj.Location = LastLocation + Vector(dx, dy, dz)

    -- local rx = 0
    -- local ry = 0
    -- local rz = math.random() * 3.14 * 2
    -- obj.Rotation = Rotator(rx,ry,rz)

    -- ���� ũ�� (��: 0.5~2.0)
    local sx = math.random() * 3 + 2
    local sy = math.random() * 3 + 2
    local sz = 1
    -- local sz = math.random() * 1.5 + 0.5

    obj.Scale = Vector(sx, sy, sz)
end

function OnOverlap(OtherActor)
end

function Tick(dt)
    -- obj.Location = obj.Location + Vector(0.1,0,0)
    -- obj.Location = obj.Location + obj.Velocity * dt
    -- obj:PrintLocation()
end
