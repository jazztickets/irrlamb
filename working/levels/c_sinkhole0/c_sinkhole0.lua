-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display zone message
function OnHitZone(HitType, Zone, HitObject)

	if HitObject == Player then
		Level.Lose("You fell down the hole!")
	else

		-- Check for active orbs
		HitObjectTemplate = Object.GetTemplate(HitObject)
		if HitObjectTemplate == tOrb then
			OrbState = Orb.GetState(HitObject)
			if OrbState == 0 then
				Level.Lose("You lost an orb!")
				return 0
			end
		end

		Object.SetLifetime(HitObject, 2)
	end

	return 0
end

-- Set up goal
GoalCount = 6

-- Set up templates
tOrb = Level.GetTemplate("orb")
