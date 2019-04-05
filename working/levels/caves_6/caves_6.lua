-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		WinState()
	end
end

-- Display zone message
function OnHitZone(HitType, Zone, HitObject)

	if HitObject == Player then
		Level.Lose("You fell off the tower!")
	else

		-- Check for active orbs
		HitObjectTemplate = Object.GetTemplate(HitObject)
		if HitObjectTemplate == tOrb then
			OrbState = Orb.GetState(HitObject)
			if OrbState == 0 then
				Level.Lose("You lost an orb!")
				return 0
			end
		end

		-- Delete objects
		Name = Object.GetName(HitObject)
		if Name ~= "drum" then
			Object.SetLifetime(HitObject, 2)
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

-- Set wrecking ball in motion
function Drop()
	Object.SetSleep(oTether1, 0)
	Object.SetSleep(oBall1, 0)
end

-- Drop orb1
function BallDrop1()
	Level.CreateObject("orb1", tOrb, 0, 5, 5)
	Timer.DelayedFunction("BallDrop2", 10.0)
	Object.SetAngularVelocity(oDrum, 0, 0.55, 0)
end

-- Drop orb2
function BallDrop2()
	Level.CreateObject("orb2", tOrb, 5, 5, 5)
	Timer.DelayedFunction("BallDrop3", 10.0)
	Object.SetAngularVelocity(oDrum, 0, 0.6, 0)
end

-- Drop orb3
function BallDrop3()
	Level.CreateObject("orb3", tOrb, -4, 5, 0)
	Timer.DelayedFunction("BallDrop4", 10.0)
	Object.SetAngularVelocity(oDrum, 0, 0.65, 0)
end

-- Drop orb4
function BallDrop4()
	Level.CreateObject("orb4", tOrb, -3, 5, 2)
	Object.SetAngularVelocity(oDrum, 0, 0.7, 0)
end

-- Handle end game
function WinState()
	GUI.TutorialText("You have conquered the evil twin balls!", 8)
	Object.SetSleep(oBall0, 1)
	Object.SetSleep(oBall1, 1)
	Object.SetSleep(oTether0, 1)
	Object.SetSleep(oTether1, 1)
	Object.Stop(oBall0)
	Object.Stop(oBall1)
	Object.Stop(oTether0)
	Object.Stop(oTether1)
	Object.Stop(oDrum)
	Timer.DelayedFunction("Won", 10)
end

-- End game
function Won()
	Level.Win()
end

-- Decrease drum size
function UpdateDrum()
	DrumSize = DrumSize - 0.01
	if DrumSize > 7 then
		Timer.DelayedFunction("UpdateDrum", DrumChangePeriod)
	end

	Object.SetScale(oDrum, DrumSize, DrumSize, 20)
	Object.SetShape(oDrum, DrumSize, 40, DrumSize)
end

-- States
GoalCount = 5
Crashed = 0
DrumSize = 20
DrumChangePeriod = 0.05

-- Set up templates
tOrb = Level.GetTemplate("orb")
tBox = Level.GetTemplate("box")

-- Get objects
oDrum = Object.GetPointer("drum")
oTether0 = Object.GetPointer("tether0")
oBall0 = Object.GetPointer("ball0")
oTether1 = Object.GetPointer("tether1")
oBall1 = Object.GetPointer("ball1")
Object.SetSleep(oTether0, 0)
Object.SetSleep(oBall0, 0)

-- Build block stack
BaseCount = 3
Y = 0.5
while BaseCount >= 0 do
	for i = 0, BaseCount do
		Level.CreateObject("box", tBox, -BaseCount / 2 + i, Y, 0)
	end
	BaseCount = BaseCount - 1
	Y = Y + 1
end

-- Set drop timer
Timer.DelayedFunction("UpdateDrum", DrumChangePeriod)
Timer.DelayedFunction("BallDrop1", 10.0)
Timer.DelayedFunction("Drop", 13)
--Timer.DelayedFunction("WinState", 2.3)
