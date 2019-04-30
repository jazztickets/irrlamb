-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Player collision handler
function OnHitPlayer(PlayerObject, OtherObject)

	-- Handle losing
	if OtherObject == oPlane then
		Level.Lose("Only cubes may touch the mighty plane.")
	else
		local Template = Object.GetTemplate(OtherObject)
		if Template == tOrb then
			if OrbsTouched[OtherObject] == nil then
				Timer.Callback("DeleteOrb", 2.1)
				OrbDeleted = OtherObject
				OrbsTouched[OtherObject] = 1
			end
		end
	end
end

-- Delete an orb
function DeleteOrb()
	if OrbDeleted ~= nil then
		local X, Y, Z = Object.GetPosition(OrbDeleted)
		Audio.Play("pop.ogg", X, Y, Z, 0, 0.0, 0.5, 1, 2)
		Object.Delete(OrbDeleted)
	end
end

-- Set up goal
GoalCount = 5
OrbDeleted = nil

-- Set up templates
tOrb = Level.GetTemplate("orb")
tSlab0 = Level.GetTemplate("box_slab0")
tSlab1 = Level.GetTemplate("box_slab1")
tThin0 = Level.GetTemplate("box_thin0")

-- Set up objects
oPlane = Object.GetPointer("plane")
OrbsTouched = {}

-- Set up thin slabs
for i = 1, 10 do
	if i == 10 then
		Level.CreateObject("slab1_" .. i, tSlab1, 18 + i * 3, 8.5, 0)
	else
		Level.CreateObject("slab0_" .. i, tSlab0, 18 + i * 3, 8, 0)
	end
end

-- Set up pillars
Spacing = 1.5
X = 190
Y = 8
Z = -4 * Spacing
for i = 1, 81 do
	if i > 9 * 3 or Z == 0 then
		Level.CreateObject("thin0_" .. i, tThin0, X, Y, Z)
	end
	if i == 28 or i == 36 or i == 73 or i == 81 then
		Level.CreateObject("orb_" .. i, tOrb, X, Y + 8.5, Z)
	end

	Z = Z + Spacing
	if i % 9 == 0 then
		Z = -4 * Spacing
		X = X + Spacing
	end
end
