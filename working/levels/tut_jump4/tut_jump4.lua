
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

Level.CreateObject("box", tCrate, 0, -3, 14)
Level.CreateObject("box", tCrate, 0, -1, 14)

Level.CreateObject("box", tCrate, 3, -3, 16)
Level.CreateObject("box", tCrate, 3, -1, 16)

Level.CreateObject("box", tCrate, 7, -3, 16)
Level.CreateObject("box", tCrate, 7, -1, 16)

Level.CreateObject("box", tCrate, 11, -3, 16)
Level.CreateObject("box", tCrate, 11, -1, 16)

Level.CreateObject("box", tCrate, 11, -3, 12)
Level.CreateObject("box", tCrate, 11, -1, 12)

Level.CreateObject("box", tCrate, 11, -3, 8)
Level.CreateObject("box", tCrate, 11, -1, 8)

Level.CreateObject("box", tCrate, 11, -3, 4)
Level.CreateObject("box", tCrate, 11, -1, 4)

-- Show text
GUI.Text("The key to this one is taking one jump at a time. When you jump, briefly hold [" .. KEY_BACK .."] in the air to land stopped.", 20)
