-- The delivery delay is saved with the game, so what it was before this changed it has to be too.
function InstantDeliveryScript:StartScript()
	self:Apply();
end

function InstantDeliveryScript:EndScript()
	local activity = ActivityMan:GetActivity();
	if activity then
		local originalDelay = activity:LoadNumber("InstantDeliveryScript_OriginalDelay");
		if originalDelay ~= 0 then
			ToGameActivity(activity).DeliveryDelay = originalDelay;
			activity:SaveNumber("InstantDeliveryScript_OriginalDelay", 0);
		end
	end
end

function InstantDeliveryScript:UpdateScript()
	self:Apply();
end

function InstantDeliveryScript:Apply()
	local activity = ActivityMan:GetActivity();
	if activity and activity:LoadNumber("InstantDeliveryScript_OriginalDelay") == 0 then
		local gameActivity = ToGameActivity(activity);
		if gameActivity then
			activity:SaveNumber("InstantDeliveryScript_OriginalDelay", gameActivity.DeliveryDelay);
			gameActivity.DeliveryDelay = 1;
		end
	end
end
