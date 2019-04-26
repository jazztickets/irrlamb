-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Player collision handler
function OnHitPlayer(PlayerObject, OtherObject)

	-- Handle losing
	if OtherObject == oPlane then
		Level.Lose("Spheres can't touch the mighty plane.")
	end
end

-- Set up goal
GoalCount = 5

-- Set up templates
tOrb = Level.GetTemplate("orb")
oPlane = Object.GetPointer("plane")
