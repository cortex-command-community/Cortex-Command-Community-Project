function Create(self)
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
	
	for att in self.Attachables do
		att.ForcedHFlip = self.ForcedHFlip;
	end
end

function ThreadedUpdate(self)
	-- keep anything from moving us
	self.Pos = self.pinPos;
	
	if not self.Turret or self.Turret.MountedDevice.PresetName == "" then -- for some reason, turrets return a null Entity rather than just nil if they have no mounted device
		self:GibThis();
	end
end