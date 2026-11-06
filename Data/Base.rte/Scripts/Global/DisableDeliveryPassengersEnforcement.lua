function DisableDeliveryPassengersEnforcementScript:StartScript()
	self:SetEnforced(false);
end

function DisableDeliveryPassengersEnforcementScript:EndScript()
	self:SetEnforced(true);
end

function DisableDeliveryPassengersEnforcementScript:UpdateScript()
	self:SetEnforced(false);
end

function DisableDeliveryPassengersEnforcementScript:SetEnforced(enforced)
	local activity = ActivityMan:GetActivity();
	if activity then
		activity = ToGameActivity(activity);
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if activity:PlayerActive(player) then
				local buyMenu = activity:GetBuyGUI(player);
				if buyMenu then
					buyMenu.EnforceMaxPassengersConstraint = enforced;
				end
			end
		end
	end
end
