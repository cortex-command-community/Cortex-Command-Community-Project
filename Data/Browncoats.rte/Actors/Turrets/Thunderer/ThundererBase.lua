function Create(self)
	if self:GetNumberValue("KeepUnflipped") == -1 then
		self.keepFlipped = true;
	else
		self.keepFlipped = false;
	end
	
	-- saving and loading can mess this up, so handle it:
	
	self.pinPos = Vector(self.Pos.X, self.Pos.Y);
	
	if self:NumberValueExists("ThundererPinPosX") then
		self.pinPos.X = self:GetNumberValue("ThundererPinPosX");
		self.pinPos.Y = self:GetNumberValue("ThundererPinPosY");
	else
		self:SetNumberValue("ThundererPinPosX", self.pinPos.X);
		self:SetNumberValue("ThundererPinPosY", self.pinPos.Y);
	end

	self.AIMode = Actor.AIMODE_SENTRY;
end

function Update(self)
	-- keep anything from moving us
	self.Pos = self.pinPos;

	self.HFlipped = self.keepFlipped;
end