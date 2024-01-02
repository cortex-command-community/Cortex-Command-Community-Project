function InstantDeliveryScript:StartScript()
	local activity = ActivityMan:GetActivity();
	if activity then
		local gameActivity = ToGameActivity(activity);
		if gameActivity then
			gameActivity.DeliveryDelay = 1;
			self:Deactivate();
		end
	end
end