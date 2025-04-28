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
    -- ESC 누를 시 버튼 표시/숨기기 전환
    if Input:GetKeyDown(EKeys.Esc) and buttonComp then
        buttonComp.IsVisible = not buttonComp.IsVisible
    end
end

function OnButtonPressed()
    LoadScene(0)
end