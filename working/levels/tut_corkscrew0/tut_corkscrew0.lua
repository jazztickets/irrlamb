
-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Set up goal
GoalCount = 1

-- Show text
GUI.Text("Start holding [" .. KEY_LEFT .. "] as you climb up the ramp. This will move you to the right when you're upside down.", 10)
