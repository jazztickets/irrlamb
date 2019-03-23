-- Display tutorial text
function ShowMoreText()
	GUI.TutorialText("Hit [" .. KEY_RESET .."] to restart the level.", 15)
end

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

oLog = Level.CreateObject("log0", tLog, 0, -2.5, 8.1, 0, 0, 0)
Level.CreateConstraint("constraint0", tConstraint, oLog, 0)
Object.SetAngularVelocity(oLog, 0, 0, 2)

-- Show text
GUI.TutorialText("When rolling on the cylinder, tap [" .. KEY_RIGHT .. "] to counteract the rotation.", 10)
Timer.DelayedFunction("ShowMoreText", 15)
