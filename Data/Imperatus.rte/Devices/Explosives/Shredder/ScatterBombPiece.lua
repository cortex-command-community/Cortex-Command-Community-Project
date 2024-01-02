function Create(self)
	self.detTimer = Timer();
	self.detTime = self.Sharpness;
end

function Update(self)
	if self.detTimer:IsPastSimMS(self.detTime) then
		self:GibThis();
	end
end
