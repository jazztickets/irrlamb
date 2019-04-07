-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display lose message
function OnHitZone(HitType, Zone, Object)
	
	if Object == Player then
		Level.Lose()
		return 1
	end
	
	return 0
end


-- Set up camera
Camera.SetYaw(-90)
Camera.SetPitch(5)

-- Set up goal
GoalCount = 1

-- Show text
GUI.Text("Jump and hold [" .. KEY_RIGHT .. "] at the same time. While still holding [" .. KEY_RIGHT .. "], jump again right as you land.", 15)
