-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
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
	oOrb1 = Level.CreateObject("orb1", tOrb, 0, 5, 5)
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

-- Set up goal
GoalCount = 5

-- Set drop timer
Timer.DelayedFunction("UpdateDrum", DrumChangePeriod)
Timer.DelayedFunction("BallDrop1", 10.0)
Timer.DelayedFunction("Drop", 13)
