-- Display tutorial text
function ShowMoreText()
	GUI.TutorialText("Once you're facing the wall, Use [" .. KEY_LEFT .. "] and [" .. KEY_RIGHT .. "] to move side to side.", 30)
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
GUI.TutorialText("Try moving your camera to the side. This way you can see how close you are to the next step.", 15)
Timer.Callback("ShowMoreText", 17)
