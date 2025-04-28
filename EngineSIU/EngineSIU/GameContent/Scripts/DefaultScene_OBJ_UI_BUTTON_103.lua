function BeginPlay()
    local buttonComp = GetUIButtonComponent(obj)
    if buttonComp then
        buttonComp:Bind(function()
            print("Hello from Lua!")
        end)
    end
end


function EndPlay()
end

function OnOverlap(OtherActor)
end

function Tick(dt)
    -- obj.Location = obj.Location + Vector(0.1,0,0)
    -- obj.Location = obj.Location + obj.Velocity * dt
    -- obj:PrintLocation()
end
