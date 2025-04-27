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
    -- 이동 입력
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

    -- 점프 입력
    if Input:GetKeyDown(EKeys.SpaceBar) and obj.Location.z <= 0 then
        Velocity.z = JumpVelocity
        IsJumping = true
    end

    -- 중력 적용
    Velocity.z = Velocity.z + Gravity * dt

    -- 위치 적용
    obj.Location = obj.Location + Vector(0, 0, Velocity.z * dt)

    -- 바닥 체크 및 정지
    if obj.Location.z <= 0 then
        obj.Location = Vector(obj.Location.x, obj.Location.y, 0)
        Velocity.z = 0
        IsJumping = false
    end

    -- Score라는 이름의 액터 찾기
    local scoreActor = FindActorByLabel("Score")
    if scoreActor then
        local textComp = GetUITextComponent(scoreActor)
        if textComp then
            local z = math.floor(obj.Location.z * 100) / 100
            textComp.Text = "Score:"..tostring(z)
        end
    end
end
