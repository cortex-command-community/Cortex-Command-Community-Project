function DisableDeliveryPassengersEnforcementScript:StartScript()
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		local activity = ActivityMan:GetActivity();
		if activity then
			activity = ToGameActivity(activity);
			if activity:PlayerActive(player) then
				local buyMenu = activity:GetBuyGUI(player);
				if buyMenu then
					buyMenu.EnforceMaxPassengersConstraint = false;
				end
			end
		end
	end
end