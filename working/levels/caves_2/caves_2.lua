
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
	oOrb = Level.CreateObject("orb", tOrb, 0, 8, 80, 0, 0, 0)

	GUI.Text("At last! Now exit the cave.", 5)
end

-- Display lose message
function OnHitZone(HitType, Zone, HitObject)
	if HitObject == Player then
		Level.Lose("You fell into hot lava!")
		return 1
	end

	return 0
end

-- Set up goal
GoalCount = 5
