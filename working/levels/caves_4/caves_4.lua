-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display lose message
function OnHitZone(HitType, Zone, HitObject)

	if HitObject == Player then
		Level.Lose()
	else
		Object.SetLifetime(HitObject, 2)
	end

	return 0
end

-- Display lose message
function OnHitPlayer(PlayerObject, OtherObject)

	if OtherObject == Collision then
		Level.Lose("You fell off the raft!")
	end
end

-- Set up goal
GoalCount = 5

-- Get static collision object
Collision = Object.GetPointer("collision")

-- Set up camera
Camera.SetYaw(0)
--Camera.SetPitch(0)
