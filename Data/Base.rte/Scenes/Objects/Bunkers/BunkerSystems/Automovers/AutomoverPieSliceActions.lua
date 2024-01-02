function CommandDisplayInfoUI(self)
	if self:NumberValueExists("DisplayInfoUI") then
		self:RemoveNumberValue("DisplayInfoUI");
	else
		self:SetNumberValue("DisplayInfoUI", 1);
	end
end

function CommandSwapAutomoverAcceptsAllTeams(self)
	self:SetNumberValue("SwapAcceptsAllTeams", 1);
end

function CommandSwapAutomoverAcceptsCrafts(self)
	self:SetNumberValue("SwapAcceptsCrafts", 1);
end

function CommandSwapAutomoverHumansRemainUpright(self)
	self:SetNumberValue("SwapHumansRemainUpright", 1);
end

function CommandModifyAutomoverSpeed(self)
	self:RemoveNumberValue("ModifyMassLimit");
	if self:NumberValueExists("ModifyMovementSpeed") then
		self:RemoveNumberValue("ModifyMovementSpeed");
	else
		self:SetNumberValue("ModifyMovementSpeed", 1);
	end
end

function CommandModifyAutomoverMassLimit(self)
	self:RemoveNumberValue("ModifyMovementSpeed");
	if self:NumberValueExists("ModifyMassLimit") then
		self:RemoveNumberValue("ModifyMassLimit");
	else
		self:SetNumberValue("ModifyMassLimit", 1);
	end
end

function CommandModifyAutomoverVisualEffectsType(self)
	self:SetNumberValue("ModifyVisualEffectsType", 1);
end

function CommandModifyAutomoverVisualEffectsSize(self)
	self:SetNumberValue("ModifyVisualEffectsSize", 1);
end

function CommandActorLeaveAutomoverNetwork(self)
	self:SetNumberValue("Automover_LeaveAutomoverNetwork", 1);
end

function CommandActorChooseTeleporter(self)
	self:SetNumberValue("Automover_ChooseTeleporter", 1);
end