-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 2 then
		Level.CreateObject("orb", tOrb, 4, 2, 2, 0, 0, 0);
	elseif GoalCount == 1 then
		Level.CreateObject("orb", tOrb, 1, 2, -5, 0, 0, 0);
	elseif GoalCount == 0 then
		Level.Win()
	end
end

-- Display lose message
function OnHitPlayer(PlayerObject, OtherObject)
	if OtherObject == oLava then
		Level.Lose("You fell into hot lava!")
	end
end

-- Display lose message
function OnHitZone(HitType, Zone, HitObject)

	if HitObject == Player then
		Level.Lose("You fell off!")
	else
		Object.SetLifetime(HitObject, 2)
	end

	return 0
end

-- Set up objects
tOrb = Level.GetTemplate("orb")
oLava = Object.GetPointer("lava")

-- Set up goal
GoalCount = 3

