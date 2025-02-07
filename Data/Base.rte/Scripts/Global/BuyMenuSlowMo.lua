function BuyMenuSlowMoScript:StartScript()
	self.SlowMo = false;
	self.Change = false;
	self.originalTimeScale = TimerMan.TimeScale
	self.originalDeltaTimeSecs = TimerMan.DeltaTimeSecs + 0
end

function BuyMenuSlowMoScript:UpdateScript()
	local activity = ActivityMan:GetActivity();
	if activity then
		activity = ToGameActivity(activity);

		-- Check if the buy menu is open
		if activity:IsBuyGUIVisible(-1) then
			if not self.SlowMo then
				self.SlowMo = true;
				self.Change = true;
			end
		else
			if self.SlowMo then
				self.SlowMo = false;
				self.Change = true;
			end
		end
	end

	if self.Change then
		if self.SlowMo then
			-- Slow down time
			TimerMan.DeltaTimeSecs = self.originalDeltaTimeSecs * 0.05
			TimerMan.TimeScale = 0.1
		else
			-- Return time to normal
			TimerMan.DeltaTimeSecs = self.originalDeltaTimeSecs
			TimerMan.TimeScale = self.originalTimeScale
		end
		self.Change = false;
	end
end