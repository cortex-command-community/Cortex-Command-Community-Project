function LongDeliveryScript:StartScript()
	local activity = ActivityMan:GetActivity();
	if activity then
		local gameActivity = ToGameActivity(activity);
		if gameActivity then
			gameActivity.DeliveryDelay = 10000;
			self:Deactivate();
		end
	end
end