
-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 1 then
		ShowLastOrb()
	elseif GoalCount == 0 then
		Level.Win()
	end
end

-- Spawn last orb
function ShowLastOrb()
	tOrb = Level.GetTemplate("orb")
	oOrb = Level.CreateObject("orb_cave", tOrb, -48.220, -21.117, -60.686, 0, 0, 0)

	GUI.Text("Enter the cave to begin your adventure.", 12)
end

-- Display lose message
function OnHitZone(HitType, Zone, HitObject)

	if HitObject == Player then
		Level.Lose("You flew off into space!")
		return 1
	else
		Object.SetLifetime(HitObject, 2)
	end

	return 0
end

-- Set up goal
GoalCount = 5
