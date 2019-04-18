-- Called when an orb is deactivated
function OnOrbDeactivate()
	UpdateState()
end

-- Progress the goal
function UpdateState()
	if State == 0 then
		GUI.Text("Wakey wakey! Time to get dressed for work!", 10)
		Level.CreateObject("orb", tOrb, 83.46, 53.7, -57.74, 0, 0, 0);
	elseif State == 1 then
		GUI.Text("Oh wait, almost forgot to shower first.", 8)
		ShowerSound = Audio.Play("shower.ogg", -6.019, 55.26, 45.16, 1, 0.0, 0.7, 8.0, 1.0)
		Level.CreateObject("orb", tOrb, -14.36, 64.94, 45.40, 0, 0, 0);
	elseif State == 2 then
		Audio.Stop(ShowerSound)
		GUI.Text("Alright, enough splish splashing around. Let's brush some teeth.", 10)
		Level.CreateObject("orb", tOrb, -12.53, 65.70, 8.46, 0, 0, 0);
	elseif State == 3 then
		GUI.Text("Oops, toilet needs to be flushed. Gross.", 15)
	elseif State == 4 then
		GUI.Text("Breakfast time!", 7)
		Level.CreateObject("orb", tOrb, -35.75, 25.961304, 33.431503, 0, 0, 0);
	elseif State == 5 then
		GUI.Text("Let's watch some sweet videos while we eat.", 15)
		Level.CreateObject("orb", tOrb, 41.8906, 7.574116, 40.508533, 0, 0, 0);
		X, Y, Z = Object.GetPosition(oSoccer);
		if Y > 3 then
			Object.SetLinearVelocity(oSoccer, 20, 0, 0);
		end
	elseif State == 6 then
		GUI.Text("Ok, enough of that.", 5)
		Level.CreateObject("orb", tOrb, 39.859116, 8.510316, 6.891754, 0, 0, 0);
	elseif State == 7 then
		GUI.Text("It's getting cold in here. Let's check on that furnace.", 10)
		Level.CreateObject("orb", tOrb, -37.7854, 22.8, -57.108673, 0, 0, 0);
	elseif State == 8 then
		Timer.Callback("WindowMessage", 7)
		Level.CreateObject("orb", tOrb, 97.503487, 77.7, -15.73, 0, 0, 0);
	elseif State == 9 then
		GUI.Text("Time to go! Hmm, where did I leave my keys...", 10)
		Level.CreateObject("orb", tOrb, 87.076630, 0.500000, 48.877991, 0, 0, 0);
	elseif State == 10 then
		Timer.Callback("Finished", 10)
		if Goofs == 1 then
			GoofText = Goofs .. " time."
		else
			GoofText = Goofs .. " times."
		end

		if Secrets == 1 then
			SecretsFound = Secrets .. " secret."
		else
			SecretsFound = Secrets .. " secrets."
		end

		GUI.Text("Level completed!\nYou goofed " .. GoofText .. "\nYou found " .. SecretsFound, 10)
	end

	State = State + 1
end

-- Player collision handler
function OnHitPlayer(PlayerObject, OtherObject)

	-- Handle Alan touching
	if OtherObject == oAlan then
		if AlanTouched == 0 then
			Secrets = Secrets + 1
			Audio.Play("emptyclip.ogg", -36.756924, 1.500000, -35.409771, 0, 0.5, 0.5)
			GUI.Text("Secret Alan watches you play irrlamb... even when you don't want him to. " .. SecretText(), 13)
			AlanTouched = 1
		end
	end

	-- Handle soccer ball riding
	if SoccerRode == 0 then
		if OtherObject == oSoccer then
			if SoccerStartTimer == 0 then
				SoccerStartTimer = Timer.Stamp()
			end
		elseif OtherObject == oHouse and SoccerStartTimer > 0 then
			SoccerRideTime = Timer.Stamp() - SoccerStartTimer
			SoccerRode = 1
			if SoccerRideTime > 2.0 and State >= 4 then
				GUI.Text("You rode the ball for " .. string.format("%.3f", SoccerRideTime) .. " seconds. Nice!", 7)
			end
		end
	end
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

	-- Handle collision
	if HitObject == Player then
		if ZoneName == "zone_toilet_handle" then
			Audio.Play("flush.ogg", -8.84, 55.95, 31.62, 0, 0.0, 1.0, 10)
			if State == 4 then
				UpdateState()
			end
		elseif ZoneName == "zone_toilet" then
			Goofs = Goofs + 1
			GUI.Text("Oh man you fell into the toilet!", 6)
			Audio.Play("fart.ogg", -9, 56, 31, 0, 0.0, 0.8, 10)
			return 1
		elseif ZoneName == "zone_stairs" then
			if State < 5 then
				if DownstairsAttempt >= DownstairsAttemptMax then
					GUI.Text("Fine, go downstairs. Good luck getting back up!", 10)
					return 1
				else
					Object.Stop(Player)
					Object.SetPosition(Player, -9.255825, 60, -51.385429)
					GUI.Text("You're not ready to go downstairs yet, doog!", 5)
					if DownstairsAttempt == 0 then
						Goofs = Goofs + 1
					end

					DownstairsAttempt = DownstairsAttempt + 1
				end
			end
		elseif ZoneName == "zone_furnace" then
			FurnaceTeleport()
		elseif ZoneName == "zone_poo" then
			Secrets = Secrets + 1
			GUI.Text("You touched the secret poo! " .. SecretText(), 10)
			return 1
		elseif ZoneName == "zone_tv" then
			GUI.Text("Hey, how'd you make it up here?", 7)
			Timer.Callback("SecretLevel", 10)
			return 1
		elseif ZoneName == "zone_smells" then
			GUI.Text("Smells funky in here...", 7)
			return 1
		elseif ZoneName == "zone_lint" then
			GUI.Text("Lots of lint back here.", 7)
			return 1
		elseif ZoneName == "zone_choria" then
			GamesPlayed = GamesPlayed + 1
			GUI.Text("Hey this game looks neat. I love chores.", 7)
			Timer.Callback("CheckGames", 8)
			return 1
		elseif ZoneName == "zone_emptyclip" then
			GamesPlayed = GamesPlayed + 1
			GUI.Text("That game was okay back in 2006...", 7)
			Timer.Callback("CheckGames", 8)
			return 1
		elseif ZoneName == "zone_pizza" then
			Secrets = Secrets + 1
			GUI.Text("You licked the secret pizza! " .. SecretText(), 10)
			return 1
		end
	elseif HitName == "salt" then
		if Spilt == 0 then
			Goofs = Goofs + 1
			GUI.Text("Oh god you spilled the salt...", 5)
			Spilt = 1
		end
	end

	return 0
end

-- Level completed
function Finished()
	Level.Win()
end

-- Get text when secrets are found
function SecretText()
	return Secrets .. " out of " .. TotalSecrets .. " secrets found."
end

-- Secret level text
function SecretLevel()
	GUI.Text("Oh man I don't feel so good anymore...", 7)
	Timer.Callback("SecretLevelLaunch", 10)
end

-- Secret level launch
function SecretLevelLaunch()
	Level.Change(1, 1)
end

-- Check games played count
function CheckGames()
	if GamesPlayed >= 2 then
		Audio.Stop(aSong)
		GUI.Text("What was that noise?", 5)
		Audio.Play("emptyclip.ogg", -36.756924, 1.5, -35.409771, 0, 0.2, 0.2)
		oAlan = Level.CreateObject("alan", tAlan, -36.756924, 1.5, -35.409771, 180, 180, 0);
	end
end

-- Give message after furnace teleport
function WindowMessage()
	GUI.Text("Alright, let's get these blinds shut.", 7)
end

-- Teleport player to master bedroom
function FurnaceTeleport()
	GUI.Text("You got sucked into the HVAC system. You almost died!", 6)
	Object.Stop(Player)
	Object.SetPosition(Player, 87.756561, 100, 36.694031)
	Camera.SetYaw(180)
end

-- Objects
tOrb = Level.GetTemplate("orb")
tAlan = Level.GetTemplate("alan")
oSoccer = Object.GetPointer("soccer")
oHouse = Object.GetPointer("house")
oAlan = nil
ShowerSound = nil
Spilt = 0
SoccerStartTimer = 0
SoccerRode = 0
GamesPlayed = 0
DownstairsAttempt = 0
DownstairsAttemptMax = 20
AlanTouched = 0

-- Sounds
aSong = Audio.Play("jazztown.ogg", 86, 72, 36, 1, 0.0, 1.0, 20.0, 20.0)
Audio.Play("furnace.ogg", -37, 11, -56, 1, 0.0, 1.0, 20.0, 10.0)

-- Set up goal
TotalSecrets = 3
Secrets = 0
Goofs = 0
State = 0
UpdateState()
