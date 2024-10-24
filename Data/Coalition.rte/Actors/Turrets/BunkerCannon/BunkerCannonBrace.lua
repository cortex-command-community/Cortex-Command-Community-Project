function Create(self)
	-- saving and loading can mess this up, so handle it:
	
	self.pinPos = Vector(self.Pos.X, self.Pos.Y);
	
	if self:NumberValueExists("BunkerCannonPinPosX") then
		self.pinPos.X = self:GetNumberValue("BunkerCannonPinPosX");
		self.pinPos.Y = self:GetNumberValue("BunkerCannonPinPosY");
	else
		self:SetNumberValue("BunkerCannonPinPosX", self.pinPos.X);
		self:SetNumberValue("BunkerCannonPinPosY", self.pinPos.Y);
	end

	self.AIMode = Actor.AIMODE_SENTRY;
	
	if self:GetNumberValue("KeepUnflipped") == -1 then
		self.keepFlipped = true;
	else
		self.keepFlipped = false;
	end
	
	if self.Turret then
		if self.Turret.MountedDevice then
			self.Turret.MountedDevice:SendMessage("SetKeepUnflipped", self:GetNumberValue("KeepUnflipped"));
		end
	end
end

function Update(self)
	-- keep anything from moving us
	self.Pos = self.pinPos;

	self.HFlipped = self.keepFlipped;
end