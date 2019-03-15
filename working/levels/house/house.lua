-- Called when an orb is deactivated
function OnOrbDeactivate()
	UpdateState()
end

-- Progress the goal
function UpdateState()
	if State == 0 then
		GUI.TutorialText("Oh wait, almost forgot to shower first.", 8)
		ShowerSound = Audio.Play("shower.ogg", -6.019, 55.26, 45.16, 1, 0.0, 0.7, 8.0, 1.0)
		Level.CreateObject("orb", tOrb, -14.36, 64.94, 45.40, 0, 0, 0);
	elseif State == 1 then
		Audio.Stop(ShowerSound)
		GUI.TutorialText("Alright, enough splish splashing around. Let's brush some teeth.", 10)
		Level.CreateObject("orb", tOrb, -12.53, 65.70, 8.46, 0, 0, 0);
	elseif State == 2 then
		GUI.TutorialText("Oops, toilet needs to be flushed. Gross.", 15)
	elseif State == 3 then
		GUI.TutorialText("Breakfast time!", 7)
		Level.CreateObject("orb", tOrb, -27.988945, 25.961304, 33.431503, 0, 0, 0);
	end

	State = State + 1
end

-- Zone handler
function OnHitZone(HitType, Zone, HitObject)

	-- Dismiss if exiting zone
	if HitType ~= 0 then
		return 0
	end

	-- Get object attributes
	ZoneName = Object.GetName(Zone)
	HitName = Object.GetName(HitObject)
	X, Y, Z = Object.GetPosition(Zone)

	-- Handle collision
	if HitObject == Player then
		if ZoneName == "zone_toilet_handle" then
			Audio.Play("flush.ogg", -8.84, 55.95, 31.62, 0, 0.0, 1.0, 10)
			if State == 3 then
				UpdateState()
			end
		elseif ZoneName == "zone_stairs" then
			if State < 4 then
				Object.Stop(Player)
				Object.SetPosition(Player, -9.255825, 60, -51.385429)
				GUI.TutorialText("You're not ready to go downstairs yet, doog!", 5)
			end
		end
	elseif HitName == "salt" then
		if Spilt == 0 then
			GUI.TutorialText("Oh god you spilled the salt...", 5)
			Spilt = 1
		end
	end

	return 0
end

-- Objects
tOrb = Level.GetTemplate("orb")
ShowerSound = nil
Spilt = 0

-- Show text
GUI.TutorialText("Wakey wakey! Time to get dressed for work!", 10)

-- Set up goal
State = 0
--UpdateState()
