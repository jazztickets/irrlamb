-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display lose message
function OnHitZone(HitType, Zone, HitObject)

	if HitObject == Player then
		Level.Lose()
		return 1
	else
		Object.SetLifetime(HitObject, 2)
	end

	return 0
end

-- Set up level
tLog = Level.GetTemplate("log")

oLog = Level.CreateObject("log0", tLog, 2, 1.25, 2.5, 90, 0, 0)
Object.SetAngularVelocity(oLog, 0, 0, 0)

oLog = Level.CreateObject("log1", tLog, -20, 1.25, 10.5, 90, 0, 0)
Object.SetAngularVelocity(oLog, 0, 0, 0)

oLog = Level.CreateObject("log2", tLog, -22, 1.25, 18.5, 90, 0, 0)
Object.SetAngularVelocity(oLog, 0, 0, -1)

-- Set up goal
GoalCount = 3
