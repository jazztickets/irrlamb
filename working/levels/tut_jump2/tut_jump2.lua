
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
tCrate = Level.GetTemplate("crate")

Level.CreateObject("box", tCrate, 0, -3, 6)
Level.CreateObject("box", tCrate, 0, -1, 6)

Level.CreateObject("box", tCrate, 0, -3, 10)
Level.CreateObject("box", tCrate, 0, -1, 10)

-- Show text
GUI.Text("After jumping, try tapping [" .. KEY_BACK .. "] to spin backwards. This helps you land with control.", 20)
