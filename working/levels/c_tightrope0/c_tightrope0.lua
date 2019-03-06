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

-- Set up goal
GoalCount = 5

-- Set up level
tRailBig = Level.GetTemplate("rail_big")
tRailSmall = Level.GetTemplate("rail_small")
tOrb = Level.GetTemplate("orb")
BigOffset = 0.6
SmallOffset = 0.55

-- First
X = 0
Y = 0
Z = 0
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 0, 0);

X = X - 4
Y = Y - 2
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 90, 0);

Z = Z + 4
Y = Y - 2
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 0, 0);
Level.CreateObject("orb0", tOrb, X + 2.5, Y + BigOffset, Z, 0, 0, 0);

-- Second
X = X - 10
Y = Y - 1
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 0, 0);

Y = Y - 5
X = X - 5
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 0, 0);

Y = Y - 5
Z = Z - 5
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 90, 0);
Level.CreateObject("orb1", tOrb, X, Y + BigOffset, Z - 2.5, 0, 0, 0);

-- Third
X = X + 4
Y = Y + 0.5
Level.CreateObject("rail", tRailSmall, X, Y, Z, 0, 0, 0);

X = X + 4
Y = Y - 0.5
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 90, 0);

Z = Z + 11
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 90, 0);
Level.CreateObject("orb2", tOrb, X, Y + BigOffset, Z + 2.5, 0, 0, 0);

-- Fourth
X = X + 2.5
Y = Y - 2
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 0, -45);

X = X + 4.5
Y = Y - 3
Z = Z - 1
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 0, 0);
Level.CreateObject("orb3", tOrb, X + 2.5, Y + BigOffset, Z, 0, 0, 0);

-- Fifth
X = X + 7
Y = Y + 1.5
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 0, 0);

X = X + 7
Y = Y + 1.5
Level.CreateObject("rail", tRailBig, X, Y, Z, 0, 0, 0);

X = X + 5
Level.CreateObject("rail", tRailSmall, X, Y, Z, 0, 0, 0);

X = X + 2.5
Z = Z + 2.5
Level.CreateObject("rail", tRailSmall, X, Y, Z, 0, 90, 0);

X = X + 2.5
Z = Z + 2.5
Level.CreateObject("rail", tRailSmall, X, Y, Z, 0, 0, 0);

X = X + 2.5
Z = Z + 2.5
Level.CreateObject("rail", tRailSmall, X, Y, Z, 0, 90, 0);

X = X + 2.5
Z = Z + 2.5
Level.CreateObject("rail", tRailSmall, X, Y, Z, 0, 0, 0);
Level.CreateObject("orb4", tOrb, X + 2.5, Y + SmallOffset, Z, 0, 0, 0);