-- The delivery delay is saved with the game, so what it was before this changed it has to be too.
function LongDeliveryScript:StartScript()
	self:Apply();
end

function LongDeliveryScript:EndScript()
	local activity = ActivityMan:GetActivity();
	if activity then
		local originalDelay = activity:LoadNumber("LongDeliveryScript_OriginalDelay");
		if originalDelay ~= 0 then
			ToGameActivity(activity).DeliveryDelay = originalDelay;
			activity:SaveNumber("LongDeliveryScript_OriginalDelay", 0);
		end
	end
end

function LongDeliveryScript:UpdateScript()
	self:Apply();
end

function LongDeliveryScript:Apply()
	local activity = ActivityMan:GetActivity();
	if activity and activity:LoadNumber("LongDeliveryScript_OriginalDelay") == 0 then
		local gameActivity = ToGameActivity(activity);
		if gameActivity then
			activity:SaveNumber("LongDeliveryScript_OriginalDelay", gameActivity.DeliveryDelay);
			gameActivity.DeliveryDelay = 10000;
		end
	end
end
