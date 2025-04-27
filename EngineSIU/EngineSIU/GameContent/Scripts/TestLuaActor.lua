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
        --obj.Location = obj.Location + Vector(0, 1, 0) * dt * 5.0
        local vec = Vector(obj.ForwardVector.x, obj.ForwardVector.y, obj.ForwardVector.z)
        obj:Move(vec, dt * 5.0)
    end
    if Input:GetKey(EKeys.A) then
        --obj.Location = obj.Location + Vector(1, 0, 0) * dt * 5.0
        local vec = Vector(-obj.RightVector.x, -obj.RightVector.y, -obj.RightVector.z)
        obj:Move(vec, dt * 5.0)
    end
    if Input:GetKey(EKeys.S) then
        --obj.Location = obj.Location + Vector(0, -1, 0) * dt * 5.0
        local vec = Vector(-obj.ForwardVector.x, -obj.ForwardVector.y, -obj.ForwardVector.z)
        obj:Move(vec, dt * 5.0)

    end
    if Input:GetKey(EKeys.D) then
        --obj.Location = obj.Location + Vector(-1, 0, 0) * dt * 5.0
        local vec = Vector(obj.RightVector.x, obj.RightVector.y, obj.RightVector.z)
        obj:Move(vec, dt * 5.0)


    end

    -- ���� �Է�
    if Input:GetKeyDown(EKeys.SpaceBar) and obj.Location.z <= 0 then
        Velocity.z = JumpVelocity
        IsJumping = true
    end

    -- Score��� �̸��� ���� ã��
    local scoreActor = FindActorByLabel("Score")
    if scoreActor then
        local textComp = GetUITextComponent(scoreActor)
        if textComp then
            local z = math.floor(obj.Location.z * 100) / 100
            textComp.Text = "Score:"..tostring(z)
        end
    end
end
