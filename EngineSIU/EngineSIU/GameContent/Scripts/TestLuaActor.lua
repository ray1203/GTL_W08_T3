Velocity = Vector(0, 0, 0)
JumpVelocity = 8.0
Gravity = -9.8
IsJumping = false

--- @type GameObject
obj = obj

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

function Tick(dt)
    -- �̵� �Է�
    if Input:GetKey(EKeys.W) then
        obj.Location = obj.Location + Vector(0, 1, 0) * dt * 5.0
    end
    if Input:GetKey(EKeys.A) then
        obj.Location = obj.Location + Vector(1, 0, 0) * dt * 5.0
    end
    if Input:GetKey(EKeys.S) then
        obj.Location = obj.Location + Vector(0, -1, 0) * dt * 5.0
    end
    if Input:GetKey(EKeys.D) then
        obj.Location = obj.Location + Vector(-1, 0, 0) * dt * 5.0
    end

    -- ���� �Է�
    if Input:GetKeyDown(EKeys.SpaceBar) then
        obj.Location = obj.Location + Vector(0,0,1)
        IsJumping = true
    end

    -- obj.Location = obj.Location + Vector(10,10,0)
    -- -- �߷� ����
    -- Velocity.z = Velocity.z + Gravity * dt

    -- -- ��ġ ����
    -- obj.Location = obj.Location + Vector(0, 0, Velocity.z * dt)

    -- -- �ٴ� üũ �� ����
    -- if obj.Location.z <= 0 then
    --     obj.Location = Vector(obj.Location.x,obj.Location.y,0)
    --     Velocity.z = 0
    --     IsJumping = false
    -- end

    obj:PrintLocation()
end
