Velocity = Vector(0, 0, 0)
JumpVelocity = 14.0
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
    dt = dt * 10
    if Input:GetKey(EKeys.W) then
        obj.Location = obj.Location + obj.ForwardVector * dt;
        -- obj.Velocity = Vector(obj.ForwardVector.x * dt, obj.ForwardVector.y * dt, obj.Velocity.z)

    elseif Input:GetKey(EKeys.A) then
            obj.Location = obj.Location + obj.RightVector * dt;

        -- obj.Velocity = Vector(-obj.RightVector.x * dt, -obj.RightVector.y * dt, obj.Velocity.z)

    elseif Input:GetKey(EKeys.S) then
            obj.Location = obj.Location - obj.ForwardVector * dt;

        -- obj.Velocity = Vector(-obj.ForwardVector.x * dt, -obj.ForwardVector.y * dt, obj.Velocity.z)

    elseif Input:GetKey(EKeys.D) then
            obj.Location = obj.Location - obj.RightVector * dt;

        -- obj.Velocity = Vector(obj.RightVector.x * dt, obj.RightVector.y * dt, obj.Velocity.z)
    else
        -- obj.Velocity = Vector(0.0,0.0, obj.Velocity.z)
    end

    -- obj.Location = obj.Location + Vector(1,0,0)
    -- ���� �Է�
    if Input:GetKeyDown(EKeys.SpaceBar) then
        obj.Velocity = Vector(obj.Velocity.x,obj.Velocity.y,JumpVelocity)
        IsJumping = true
    end
    obj:PrintLocation()

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
