-- Display tutorial text
function ShowMoreText()
	GUI.Text("Hit [" .. KEY_RESET .."] to restart the level.", 15, 1)
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

-- Show text
GUI.Text("When rolling on the cylinder, tap [" .. KEY_RIGHT .. "] to counteract the rotation.", 10, 1)
Timer.Callback("ShowMoreText", 15)
