-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display zone message
function OnHitZone(HitType, Zone, HitObject)

	return 0
end

-- Set up objects
tBox = Level.GetTemplate("box")

-- Create wall
X = -3.5
Y = 0.5
Z = 5
for i = 1, 3 do
	for j = 1, 6 do
		Level.CreateObject("box", tBox, X + j, Y + i - 1, Z)
	end
end

-- Set up goal
GoalCount = 1
