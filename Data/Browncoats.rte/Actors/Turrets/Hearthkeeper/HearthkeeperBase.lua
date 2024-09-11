function Create(self)
	-- saving and loading can mess this up, so handle it:
	
	self.pinPos = Vector(self.Pos.X, self.Pos.Y);
	
	if self:NumberValueExists("HearthkeeperPinPosX") then
		self.pinPos.X = self:GetNumberValue("HearthkeeperPinPosX");
		self.pinPos.Y = self:GetNumberValue("HearthkeeperPinPosY");
	else
		self:SetNumberValue("HearthkeeperPinPosX", self.pinPos.X);
		self:SetNumberValue("HearthkeeperPinPosY", self.pinPos.Y);
	end

	self.AIMode = Actor.AIMODE_SENTRY;
end

function Update(self)
	-- keep anything from moving us
	self.Pos = self.pinPos;
end