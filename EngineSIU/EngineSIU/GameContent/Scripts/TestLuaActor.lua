Velocity = Vector(0, 0, 0)
JumpVelocity = 20.0
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
    -- 이동 입력 처리 (대각선 이동 가능하도록 수정)
    local moveInput = Vector(0, 0, 0)
    dt = dt * 1000
    
    -- 전후좌우 이동 입력을 모두 처리
    if Input:GetKey(EKeys.W) then
        moveInput = moveInput + obj.ForwardVector
    end
    if Input:GetKey(EKeys.S) then
        moveInput = moveInput - obj.ForwardVector
    end
    if Input:GetKey(EKeys.A) then
        moveInput = moveInput - obj.RightVector
    end
    if Input:GetKey(EKeys.D) then
        moveInput = moveInput + obj.RightVector
    end
    
    -- 이동 입력이 있는 경우 정규화 후 속도 적용
    if moveInput.x ~= 0 or moveInput.y ~= 0 then
        moveInput = moveInput:Normalize()
        obj.Velocity = Vector(moveInput.x * dt, moveInput.y * dt, obj.Velocity.z)
    else
        -- 이동 입력이 없으면 x, y 속도를 0으로 (z는 유지)
        obj.Velocity = Vector(0, 0, obj.Velocity.z)
    end

    -- 지면 체크 : TODO : 큐브위에 있는 걸로 판정 바꿔야 함
    if obj.Location.z <= 0 then
        obj.Location = Vector(obj.Location.x, obj.Location.y, 0) -- z 위치 고정
        obj.Velocity = Vector(obj.Velocity.x, obj.Velocity.y, 0) -- z 속도 리셋
        IsJumping = false
    end

    -- 점프 입력
    if Input:GetKeyDown(EKeys.SpaceBar) and not IsJumping then
        obj.Velocity = Vector(obj.Velocity.x, obj.Velocity.y, JumpVelocity)
        IsJumping = true
    end
    
    if Input:GetKeyDown(EKeys.G) then
        obj:Instantiate("StaticMeshActor", obj.Location + obj.ForwardVector * 3);
    end
    -- 위치 출력 (디버깅용)
    -- obj:PrintLocation()
    
    -- 점수 업데이트
    local scoreActor = FindActorByLabel("Score")
    if scoreActor then
        local textComp = GetUITextComponent(scoreActor)
        if textComp then
            local z = math.floor(obj.Location.z * 100) / 100
            textComp.Text = "Score:"..tostring(z)
        end
    end
end
