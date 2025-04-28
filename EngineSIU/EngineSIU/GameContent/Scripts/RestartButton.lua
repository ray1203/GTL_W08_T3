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
    -- ESC ���� �� ��ư ǥ��/����� ��ȯ
    if Input:GetKeyDown(EKeys.Esc) and buttonComp then
        buttonComp.IsVisible = not buttonComp.IsVisible
    end
end

function OnButtonPressed()
    LoadScene(1)
end
