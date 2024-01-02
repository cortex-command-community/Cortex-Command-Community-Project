function MoreGoldScript:StartScript()
	self.multiplier = 3;
	self.lastGold = {};
	self.activity = ActivityMan:GetActivity();
	for t = Activity.TEAM_1, Activity.TEAM_4 do
		self.lastGold[t] = self.activity:GetTeamFunds(t);
	end
end

function MoreGoldScript:UpdateScript()
	for t = Activity.TEAM_1, Activity.TEAM_4 do
		local diff = self.activity:GetTeamFunds(t) - self.lastGold[t];
		if diff > 0 and diff < 20 then
			diff = diff * (self.multiplier - 1);
			self.activity:SetTeamFunds(self.activity:GetTeamFunds(t) + diff, t);
		end
		self.lastGold[t] = self.activity:GetTeamFunds(t);
	end
end