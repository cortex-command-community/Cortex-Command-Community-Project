function AutosaveScript:StartScript()
	self.autosaveTimer = Timer();
	self.autosaveTimer:SetRealTimeLimitS(60 * 3);
end

function AutosaveScript:UpdateScript()
	if self.autosaveTimer:IsPastRealTimeLimit() then
		if ActivityMan:GetActivity().AllowsUserSaving then
			ActivityMan:SaveGame("AutoSave");
		end
		self.autosaveTimer:Reset();
	end
end