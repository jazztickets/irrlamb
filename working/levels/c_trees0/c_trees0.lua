-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display lose message
function OnHitZone(HitType, Zone, HitObject)

	Name = Object.GetName(HitObject)
	if Name == "orb" or HitObject == Player then
		X, Y, Z = Object.GetPosition(HitObject)
		Audio.Play("splash.ogg", X, Y, Z, 0, 0.3, 0.7)
	end

	if HitObject == Player then
		Level.Lose("You drowned!")
		return 1
	end

	return 0
end

-- Set up level
Camera.SetYaw(25)

tLog = Level.GetTemplate("log")
tOrb = Level.GetTemplate("orb")

X = 0;
Z = -10;
oLog = Level.CreateObject("log0", tLog, X, 6, Z, 90, 0, 0);
X = X + 3;
Z = Z + 4;
oLog = Level.CreateObject("log1", tLog, X, 6, Z, 90, 0, 0);
X = X - 4;
Z = Z + 5;
oLog = Level.CreateObject("log2", tLog, X, 6, Z, 90, 0, 0);
X = X + 1;
Z = Z + 4;
oLog = Level.CreateObject("log3", tLog, X, 6, Z, 90, 0, 0);
X = X - 5;
Z = Z + 1;
oLog = Level.CreateObject("log4", tLog, X, 6, Z, 90, 0, 0);
Level.CreateObject("orb", tOrb, X, 12.5, Z, 0, 0, 0);
Level.CreateObject("orb", tOrb, -14.318, 13.974, 13.698, 0, 0, 0);
Level.CreateObject("orb", tOrb, 9.084, 13.234, 15.085, 0, 0, 0);

X = 14.457
Z = 20
Level.CreateObject("log5", tLog, X, 6, Z, 0, 0, 0);
Level.CreateObject("orb", tOrb, X, 12.5, Z, 0, 0, 0);

-- Set up goal
GoalCount = 4
