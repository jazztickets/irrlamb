-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		WinState()
	end
end

-- Display zone message
function OnHitZone(HitType, Zone, HitObject)

	if Zone == oStartZone and HitObject == Player then
		GUI.Fade(-1)
		Timer.Callback("StartTower", 1)
		return 1
	elseif Zone == oLoseZone then

		if HitObject == Player then
			Level.Lose("You fell off the tower!")
		else

			-- Check for active orbs
			local HitObjectTemplate = Object.GetTemplate(HitObject)
			if HitObjectTemplate == tOrb then
				local OrbState = Orb.GetState(HitObject)
				if OrbState == 0 then
					Level.Lose("You lost an orb!")
					return 0
				end
			end

			-- Delete objects
			local Name = Object.GetName(HitObject)
			if Name ~= "drum" then
				Object.SetLifetime(HitObject, 2)
			end
		end
	end

	return 0
end

-- Initial crash
function Crash(Object, OtherObject)
	if Crashed == 0 and OtherObject == oBall0 then
		Audio.Play("blast.ogg", 0, 0, 0, 0, 1, 1)
		Crashed = 1
	end
end

-- Create plank and attach to drum
function CreatePlank(Y, Degrees)
	local X = math.cos(math.rad(Degrees)) * PlankStart
	local Z = -math.sin(math.rad(Degrees)) * PlankStart

	-- Create plank
	local i = PlankCount + 1
	oPlanks[i] = Level.CreateObject("plank" .. i, tPlank, X, Y, Z, 0, Degrees, 0)
	Level.CreateConstraint("plank_fixed" .. i, tFixed, oDrum, oPlanks[i])

	-- Update count
	PlankCount = PlankCount + 1
end

-- Start final stage
function StartTower()
	Stage = 1
	Camera.SetYaw(0)
	Camera.SetPitch(30)
	Level.CreateObject("orb0", tOrb, 0, 0.5, 7.5)
	Object.SetPosition(Player, 0, 0.5, -8.5)
	Object.Stop(Player)
	Object.SetSleep(oTether0, 0)
	Object.SetSleep(oBall0, 0)
	Object.SetAngularVelocity(oDrum, 0, 0.5, 0)
	Object.SetRotation(oDrum, 90, 0, 0)
	Object.SetPosition(oLoseZone, 0, -14, 0)
	Timer.Callback("UpdateDrum", DrumChangePeriod)
	Timer.Callback("BallDrop1", 10.0)
	Timer.Callback("Drop", 13)

	-- Recreate block tower with correct orientation
	CreateBlocks()

	-- Remove planks
	for i = 1, #oPlanks do
		Object.Delete(oPlanks[i])
	end

	GUI.Fade(5)
end

-- Create block tower
function CreateBlocks()

	-- Remove previous blocks
	for i = 1, #oBoxes do
		Object.SetPosition(oBoxes[i], 0, 1000, 0)
		Object.Delete(oBoxes[i])
	end
	oBoxes = {}

	-- Build block stack
	BaseCount = 3
	Y = 0.5
	j = 1
	while BaseCount >= 0 do
		for i = 0, BaseCount do
			oBoxes[j] = Level.CreateObject("box", tBox, -BaseCount / 2 + i, Y, 0)
			j = j + 1
		end
		BaseCount = BaseCount - 1
		Y = Y + 1
	end

end

-- Decrease drum size
function UpdateDrum()
	DrumSize = DrumSize - 0.01
	if DrumSize > 7 then
		Timer.Callback("UpdateDrum", DrumChangePeriod)
	end

	Object.SetScale(oDrum, DrumSize, DrumSize, 20)
	Object.SetShape(oDrum, DrumSize, 10, DrumSize)
end

-- Set 2nd ball in motion
function Drop()
	Object.SetSleep(oTether1, 0)
	Object.SetSleep(oBall1, 0)
end

-- Drop orb1
function BallDrop1()
	Level.CreateObject("orb1", tOrb, 0, 5, 5)
	Timer.Callback("BallDrop2", 10.0)
	Object.SetAngularVelocity(oDrum, 0, 0.55, 0)
end

-- Drop orb2
function BallDrop2()
	Level.CreateObject("orb2", tOrb, 5, 5, 5)
	Timer.Callback("BallDrop3", 10.0)
	Object.SetAngularVelocity(oDrum, 0, 0.6, 0)
end

-- Drop orb3
function BallDrop3()
	Level.CreateObject("orb3", tOrb, -4, 5, 0)
	Timer.Callback("BallDrop4", 10.0)
	Object.SetAngularVelocity(oDrum, 0, 0.65, 0)
end

-- Drop orb4
function BallDrop4()
	Level.CreateObject("orb4", tOrb, -3, 5, 2)
	Object.SetAngularVelocity(oDrum, 0, 0.7, 0)
end

-- Handle end game
function WinState()
	GUI.Text("You have conquered the evil twin balls!", 8)
	Object.SetSleep(oBall0, 1)
	Object.SetSleep(oBall1, 1)
	Object.SetSleep(oTether0, 1)
	Object.SetSleep(oTether1, 1)
	Object.Stop(oBall0)
	Object.Stop(oBall1)
	Object.Stop(oTether0)
	Object.Stop(oTether1)
	Object.Stop(oDrum)
	Timer.Callback("Won", 10)
end

-- End game
function Won()
	Level.Win()
end

-- States
GoalCount = 5
Stage = 0
Crashed = 0
DrumSize = 20
PlankStart = 12.5
DrumChangePeriod = 0.05
PlankCount = 0
Camera.SetYaw(0)

-- Set up templates
tOrb = Level.GetTemplate("orb")
tBox = Level.GetTemplate("box")
tPlank = Level.GetTemplate("plank")
tFixed = Level.GetTemplate("fixed")
oPlanks = {}
oBoxes = {}

-- Get objects
oDrum = Object.GetPointer("drum")
oTether0 = Object.GetPointer("tether0")
oBall0 = Object.GetPointer("ball0")
oTether1 = Object.GetPointer("tether1")
oBall1 = Object.GetPointer("ball1")
oLoseZone = Object.GetPointer("losezone")
oStartZone = Object.GetPointer("startzone")

-- Build planks
Y = 0
Degrees = 0
Increments = {
	{ -9.3, 0 },
	{ 1.3, -25 },
	{ 1.3, -25 },
	{ 1.3, 15 },
	{ 0.2, 30 },
	{ 1.5, 20 },
	{ 1.1, -30 },
	{ 1.4, -20 },
}

-- Create planks
for i = 1, #Increments do
	Y = Y + Increments[i][1]
	Degrees = Degrees + Increments[i][2]
	CreatePlank(Y, Degrees)
end

-- Create block tower
CreateBlocks()
