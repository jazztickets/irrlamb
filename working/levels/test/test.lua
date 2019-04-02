-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display zone message
function OnHitZone(HitType, Zone, HitObject)

	return 0
end

-- Set up goal
GoalCount = 1

tConstraintY = Level.GetTemplate("constraint_y")
oDrum = Object.GetPointer("drum")

Level.CreateConstraint("constraint", tConstraintY, oDrum, nil)
