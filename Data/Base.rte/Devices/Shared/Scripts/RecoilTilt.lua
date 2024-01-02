function Create(self)
	self.InheritedRotAngleOffset = 0;
	self.recoilAngleSize = self:NumberValueExists("RecoilAngleSize") and math.rad(self:GetNumberValue("RecoilAngleSize")) or 0.5/math.sqrt(self.Radius);
	self.recoilAngleVariation = self:NumberValueExists("RecoilAngleVariation") and 1 - self:GetNumberValue("RecoilAngleVariation") or 0.2;
	self.recoilRevertSpeed = self:NumberValueExists("RecoilRevertSpeed") and self:GetNumberValue("RecoilRevertSpeed") * 0.01 or 0.01;
end

function ThreadedUpdate(self)
	if self.InheritedRotAngleOffset > 0 then
		self.InheritedRotAngleOffset = math.max(self.InheritedRotAngleOffset - (self.recoilRevertSpeed * (1 + math.sqrt(self.RateOfFire * 0.1) * self.InheritedRotAngleOffset)), 0);
	end
end

function OnFire(self)
	self.InheritedRotAngleOffset = self.InheritedRotAngleOffset + (self.recoilAngleSize * RangeRand(self.recoilAngleVariation, 1))/(1 + self.InheritedRotAngleOffset)^2;
end