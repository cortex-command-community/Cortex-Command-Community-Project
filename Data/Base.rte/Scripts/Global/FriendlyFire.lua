function FriendlyFireScript:StartScript()
	-- Minimum distance in pixels that the particles ignore teammates for
	self.safeDist = 100;
	self.updateTimer = Timer();
	self.updateTimer:SetSimTimeLimitMS(math.ceil(TimerMan.DeltaTimeMS * 3));
end

function FriendlyFireScript:UpdateScript()
	if self.updateTimer:IsPastSimTimeLimit() then
		for part in MovableMan.Particles do
			if part.HitsMOs and part.Team ~= -1 then
				if (part.Age * GetPPM() * part.Vel.Magnitude) * 0.001 > self.safeDist then
					part.Team = -1;
					part.IgnoresTeamHits = false;
				end
			end
		end
		self.updateTimer:Reset();
	end
end