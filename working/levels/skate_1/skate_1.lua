
-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display big air message
function OnHitZone(HitType, Zone, Object)
	
	if Object == Player then
		GUI.Text("Big Air!", 3)
	end
	
	return 0
end

-- Set up goal
GoalCount = 6

-- Set up level
Camera.SetYaw(-90)

-- Build block stack
tCrate = Level.GetTemplate("crate")
Level.CreateObject("box", tCrate, 14, 1, -15)
Level.CreateObject("box", tCrate, 14, 3, -15)
Level.CreateObject("box", tCrate, 14, 5, -15)

Level.CreateObject("box", tCrate, 14, 1, 15)
Level.CreateObject("box", tCrate, 14, 3, 15)
Level.CreateObject("box", tCrate, 14, 5, 15)