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
		--Level.CreateObject("orb", tOrb, -12.535572, 65.699379, 8.469651, 0, 0, 0);
	elseif State == 3 then
		GUI.TutorialText("Downstairs", 15)
		--Level.CreateObject("orb", tOrb, -12.535572, 65.699379, 8.469651, 0, 0, 0);
	end

	State = State + 1
end

-- Zone handler
function OnHitZone(HitType, Zone, HitObject)
	ZoneName = Object.GetName(Zone)
	X, Y, Z = Object.GetPosition(Zone)
	if HitType ~= 0 or HitObject ~= Player then
		return 0
	end

	if ZoneName == "toilet_handle" then
		Audio.Play("flush.ogg", -8.84, 55.95, 31.62, 0, 0.0, 1.0, 10)
		if State == 3 then
			print(ZoneName)
			UpdateState()
		end
	end

	return 0
end

-- Objects
tOrb = Level.GetTemplate("orb")

-- Show text
GUI.TutorialText("Wakey wakey! Time to get dressed for work!", 10)

-- Set up goal
State = 0
--UpdateState()
