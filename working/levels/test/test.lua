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
		Level.Lose("You fell off!")
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

function UpdateBall()
	Object.SetScale(oBall, 0.5, 0.5, 0.5)
	Object.SetShape(oBall, 1, 0, 0)
end

-- Set up goal
GoalCount = 5

-- Set up templates
tOrb = Level.GetTemplate("orb")
oBall = Object.GetPointer("ball")

-- Callbacks
Timer.Callback("UpdateBall", 2)
