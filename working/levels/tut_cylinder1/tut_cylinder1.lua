
-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Set up goal
GoalCount = 1

-- Set up level
tConstraint = Level.GetTemplate("constraint_z")
tLog = Level.GetTemplate("log")

for i = 0, 3 do
	oLog = Level.CreateObject("log" .. i, tLog, 0, -2.5, 8.1 + i * 10.1, 0, 0, 0)
	Level.CreateConstraint("constraint" .. i, tConstraint, oLog, nil)
	Object.SetAngularVelocity(oLog, 0, 0, 2 * (((i % 2) * 2) - 1))
end

-- Show text
GUI.Text("When rolling on the cylinders, tap [" .. KEY_LEFT .. "]  and [" .. KEY_RIGHT .. "] to counteract the rotation.", 10)
