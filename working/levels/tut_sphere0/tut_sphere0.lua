
-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display lose message
function OnHitZone(HitType, Zone, HitObject)

	Name = Object.GetName(HitObject)
	if Name == "orb" or HitObject == Player then
		X, Y, Z = Object.GetPosition(HitObject)
		Audio.Play("splash.ogg", X, Y, Z, 0, 0.3, 0.7)
	end

	if HitObject == Player then
		Level.Lose("You drowned!")
		return 1
	end

	return 0
end

-- Set up goal
GoalCount = 1

-- Show text
GUI.Text("Get near the top of the ball, then tap [" .. KEY_BACK .."] to roll the ball forward.", 10, 1)
