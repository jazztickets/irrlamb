
-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Set up goal
GoalCount = 5

-- Build block stack
tRubble = Level.GetTemplate("rubble")

for i = 0, 2 do
	for j = 0, 4 do
		Level.CreateObject("rubble", tRubble, -27 + j * 0.5, 15, 76 + i * 0.5)
	end
end
