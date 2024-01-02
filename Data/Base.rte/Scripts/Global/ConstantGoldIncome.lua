function ConstantGoldIncomeScript:StartScript()
	self.updateTimer = Timer();
	self.updateInterval = 50;
	self.goldPerSecond = 10;
	self.activity = ActivityMan:GetActivity();
end

function ConstantGoldIncomeScript:UpdateScript()
	if self.activity.ActivityState ~= Activity.EDITING and self.updateTimer:IsPastSimMS(self.updateInterval) then
		for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
			if self.activity:IsHumanTeam(team) then
				self.activity:SetTeamFunds(self.activity:GetTeamFunds(team) + (self.updateInterval / 1000 * self.goldPerSecond), team);
			end
		end
		self.updateTimer:Reset();
	end
end