-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 1 and HardMode == 1 then
		Level.CreateObject("orb", tOrb, 3, 1.5, 0, 0, 0, 0);
	elseif GoalCount == 0 then
		Level.Win()
	end
end

-- Display lose message
function OnHitZone(HitType, Zone, HitObject)

	ZoneName = Object.GetName(Zone)
	Name = Object.GetName(HitObject)

	if ZoneName == "zone_secret" then
		Timer.Callback("SecretMessage", 10)
		Timer.Callback("Secret", 15)
		SecretFound = 1
		return 1
	end

	if ZoneName == "zone_lose" then
		X, Y, Z = Object.GetPosition(HitObject)
		Audio.Play("splash.ogg", X, Y, Z, 0, 0.3, 0.7)
		if HitObject == Player then
			Level.Lose("You drowned!")
		end
	end

	return 0
end

-- Show hint
function Hint()
	if SecretFound == 0 then
		GUI.Text("Can you find the secret?", 5)
	end
end

-- Show secret message
function SecretMessage()
	GUI.Text("You found the secret level!", 5)
end

-- Launch secret level
function Secret()
	Level.Change(1, 0)
end

-- Set up level
HardMode = 0
SecretFound = 0
GoalCount = 3 + HardMode
Camera.SetPitch(75)

tOrb = Level.GetTemplate("orb")

-- Text
GUI.Text("Jump forward, then hold [" .. KEY_BACK .. "] as you land to kick the orb off and take its place.", 10)
Timer.Callback("Hint", 30)