-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display lose message
function OnHitPlayer(PlayerObject, OtherObject)
	if OtherObject == oLava then
		Level.Lose("You fell into hot lava!")
	end
end

-- Set up goal
GoalCount = 10

-- Get objects
oLava = Object.GetPointer("lava")

-- Set up camera
Camera.SetYaw(45)
Camera.SetPitch(15)
