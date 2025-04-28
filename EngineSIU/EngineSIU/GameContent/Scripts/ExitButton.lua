function BeginPlay()
    buttonComp = GetUIButtonComponent(obj)
    if buttonComp then
        buttonComp:Bind(OnButtonPressed)
    end
end


function EndPlay()
end

function OnOverlap(OtherActor)
end

function Tick(dt)
end

function OnButtonPressed()
    ExitGame()
end