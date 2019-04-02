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

		Object.SetLifetime(HitObject, 2)
	end

	return 0
end

-- Set up goal
GoalCount = 5

-- Set up templates
tOrb = Level.GetTemplate("orb")

-- Build block stack
tCrate = Level.GetTemplate("box")

BaseCount = 3
Y = 0.5
while BaseCount >= 0 do
	for i = 0, BaseCount do
		Level.CreateObject("box", tCrate, -BaseCount / 2 + i, Y, 0)
	end
	BaseCount = BaseCount - 1
	Y = Y + 1
end
