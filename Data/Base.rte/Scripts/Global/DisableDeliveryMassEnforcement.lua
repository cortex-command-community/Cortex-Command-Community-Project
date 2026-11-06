function DisableDeliveryMassEnforcementScript:StartScript()
	self:SetEnforced(false);
end

function DisableDeliveryMassEnforcementScript:EndScript()
	self:SetEnforced(true);
end

function DisableDeliveryMassEnforcementScript:UpdateScript()
	self:SetEnforced(false);
end

function DisableDeliveryMassEnforcementScript:SetEnforced(enforced)
	local activity = ActivityMan:GetActivity();
	if activity then
		activity = ToGameActivity(activity);
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if activity:PlayerActive(player) then
				local buyMenu = activity:GetBuyGUI(player);
				if buyMenu then
					buyMenu.EnforceMaxMassConstraint = enforced;
				end
			end
		end
	end
end
