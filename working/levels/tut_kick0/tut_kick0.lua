-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 1 and HardMode == 1 then
		Level.CreateObject("log0", tOrb, 3, 1.5, 0, 0, 0, 0);
	elseif GoalCount == 0 then
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

-- Set up level
HardMode = 0
GoalCount = 3 + HardMode
Camera.SetPitch(75)

tOrb = Level.GetTemplate("orb")

-- Text
GUI.TutorialText("Jump forward, then hold [" .. KEY_BACK .. "] as you land to kick the orb off and take its place.", 10)
