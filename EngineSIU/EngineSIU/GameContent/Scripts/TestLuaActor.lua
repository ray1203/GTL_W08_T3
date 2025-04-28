Velocity = Vector(0, 0, 0)
JumpVelocity = 25
RecentlyHit = false
HitCool = 3.0
CoolDT = 0
MaxVelocity = 20

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
    if RecentlyHit == false then
        local OtherVelocity = OtherActor.Velocity
        obj.Velocity = obj.Velocity + OtherVelocity
        RecentlyHit = true
    end
end

function Tick(dt)
    -- 이동 입력 처리 (대각선 이동 가능하도록 수정)
    local moveInput = Vector(0, 0, 0)

    if RecentlyHit == true then
        CoolDT = CoolDT + dt
        if CoolDT >= HitCool then
            CoolDT = 0
            RecentlyHit = false
        end
    end
    -- dt = dt * 1000
    
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
        local factor = 1
        if obj.bGrounded == false then
            factor = 0.3
        end
        obj.Velocity = obj.Velocity + Vector(moveInput.x * dt * 100 * factor, moveInput.y * dt* 100 * factor, 0)
    else
        -- 이동 입력이 없으면 x, y 속도를 0으로 (z는 유지)
        -- obj.Velocity = obj.Velocity
    end

    -- 지면 체크 : TODO : 큐브위에 있는 걸로 판정 바꿔야 함
--[[    if obj.Location.z <= 0 then
        obj.Location = Vector(obj.Location.x, obj.Location.y, 0) -- z 위치 고정
        obj.Velocity = Vector(obj.Velocity.x, obj.Velocity.y, 0) -- z 속도 리셋
        IsJumping = false
    end
    ]]

    -- 점프 입력
    if Input:GetKeyDown(EKeys.SpaceBar) and obj.bGrounded then
        obj.Velocity = obj.Velocity + Vector(0, 0, JumpVelocity * dt * 100)
        --obj.Location = obj.Location + Vector(0,0,1)
    end

    -- 속도 clamp
    local vx, vy, vz = obj.Velocity.x, obj.Velocity.y, obj.Velocity.z
    local speed2 = vx * vx + vy * vy
    local max2 = MaxVelocity * MaxVelocity

    if speed2 > max2 then
        local speed = math.sqrt(speed2)
        vx = vx / speed * MaxVelocity
        vy = vy / speed * MaxVelocity
    end
    
    obj.Velocity = Vector(vx, vy, vz)
    
    if Input:GetKeyDown(EKeys.G) then
        obj:Instantiate("Projectile", obj.Location + obj.ForwardVector * 3);
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
