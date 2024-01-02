function Create(self)
	self.checkNodesTimer = Timer();
	self.checkNodesTimer:SetSimTimeLimitMS(500)
	self.myInfoGenerated = Automovers_AddNode(self);

	self:RemoveNumberValue("shouldReaddNode");
end

function Update(self)
	if self:GetNumberValue("shouldReaddNode") > 0 then
		self.myInfoGenerated = Automovers_AddNode(self);
		self:RemoveNumberValue("shouldReaddNode");
	end

	if AutomoverData[self.Team].energyLevel <= 0 then
		self.Frame = 16;
	elseif self.myInfoGenerated and self.checkNodesTimer:IsPastSimTimeLimit() and AutomoverData[self.Team].nodeData[self] ~= nil then
		local nodeTable = AutomoverData[self.Team].nodeData[self];

		local connectsUpBits = nodeTable.connectedNodeData[Directions.Up] ~= nil and 1 or 0;
		local connectsDownBits = nodeTable.connectedNodeData[Directions.Down] ~= nil and 2 or 0;
		local connectsLeftBits = nodeTable.connectedNodeData[Directions.Left] ~= nil and 4 or 0;
		local connectsRightBits = nodeTable.connectedNodeData[Directions.Right] ~= nil and 8 or 0;

		self.Frame = connectsRightBits + connectsUpBits + connectsLeftBits + connectsDownBits;
		self.checkNodesTimer:Reset();
	end
end

function Destroy(self)
	ActivityMan:GetActivity():SetTeamFunds(ActivityMan:GetActivity():GetTeamFunds(self.Team) + self:GetGoldValue(0, 0), self.Team);
	Automovers_RemoveNode(self);
end