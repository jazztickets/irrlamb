-- Display tutorial text
function ShowMoreText()
	GUI.Text("Once you're facing the wall, Use [" .. KEY_LEFT .. "] and [" .. KEY_RIGHT .. "] to move side to side.", 30, 1)
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
GUI.Text("Try moving your camera to the side. This way you can see how close you are to the second step.", 15, 1)
Timer.Callback("ShowMoreText", 17)
