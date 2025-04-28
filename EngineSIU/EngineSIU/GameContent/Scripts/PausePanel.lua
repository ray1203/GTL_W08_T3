function BeginPlay()
    panelComp = GetUIPanelComponent(obj)
end


function EndPlay()
end

function OnOverlap(OtherActor)
end

function Tick(dt)
    if Input:GetKeyDown(EKeys.Esc) and panelComp then
        panelComp.IsVisible = not panelComp.IsVisible
    end
end

function OnButtonPressed()
end