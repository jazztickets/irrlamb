
-- Display zone message
function OnHitZone(HitType, Zone, HitObject)

	if HitObject == Player then
		Level.Win(true)
	end

	return 0
end

-- Set up templates
tBox = Level.GetTemplate("box")

-- Build pillars
for i = 0, 1000 do
	z = i * 1000 + 50 + i
	Level.CreateObject("box", tBox, -10, 25, z)
	Level.CreateObject("box", tBox, 10, 25, z)
end

