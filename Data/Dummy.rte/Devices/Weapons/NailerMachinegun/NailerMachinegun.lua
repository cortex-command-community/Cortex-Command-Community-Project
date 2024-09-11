function Create(self)
	self.muzzleTable = {Vector(14, -2), Vector(14, 1)};
	self.muzzleSelect = 0;
end

function Update(self)
	if self.FiredFrame then
		self.muzzleSelect = math.abs(self.muzzleSelect - 1);
		self.MuzzleOffset = self.muzzleTable[self.muzzleSelect + 1];
	end
	
	if self.ID == self.RootID and self.activate then
		if self.RoundInMagCount == 0 then
			self:Deactivate();
			self.activate = false;
		else
			self:Activate();
		end
	elseif self:IsActivated() then
		self.activate = true;
	else
		self.activate = false;
	end
end