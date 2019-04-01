-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display lose message
function OnHitZone(HitType, Zone, HitObject)

	if HitObject == Player then
		Level.Lose("You fell off!")
	else
		Object.SetLifetime(HitObject, 2)
	end

	return 0
end

-- Set up objects
tOrb = Level.GetTemplate("orb")
tConstraintX = Level.GetTemplate("constraint_x")
tConstraintZ = Level.GetTemplate("constraint_z")
tLog = Level.GetTemplate("log")
oOrb0 = Object.GetPointer("orb0")

Spacing = 7
Count = 5
X = 0
Y = 0
Z = 0
Attached = 0
for i = 1, Count do

	X = 0
	for j = 1, Count do
		oLog = Level.CreateObject("log", tLog, X, Y, Z, 0, 0, 0)
		Level.CreateConstraint("constraint", tConstraintX, oLog, nil)

		X = X + Spacing
	end

	Z = Z + Spacing
end

Z = Spacing / 2
for i = 1, Count do
	X = -Spacing / 2
	for j = 1, Count do
		oLog = Level.CreateObject("log", tLog, X, Y, Z, 0, 90, 0)
		Level.CreateConstraint("constraint", tConstraintZ, oLog, nil)

		X = X + Spacing
	end

	Z = Z + Spacing
end

-- Set up goal
GoalCount = 3

