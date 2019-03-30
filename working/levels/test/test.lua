-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display zone message
function OnHitZone(HitType, Zone, HitObject)

	print(HitType .. " " .. Object.GetName(Zone) .. " vs " .. Object.GetName(HitObject))

	return 0
end

-- Set up goal
GoalCount = 1

--Object.SetLinearVelocity(Player, 0, 0, 0);
--Object.SetAngularVelocity(Player, 0, 0, 20);
