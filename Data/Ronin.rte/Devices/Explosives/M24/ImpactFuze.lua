function Create(self)
	self.fuzeDelay = 3500;
	self.fuzeDecreaseIncrement = 200;
end

function Update(self)
	if self.fuze then
		if self.fuze:IsPastSimMS(self.fuzeDelay) then
			self:GibThis();
		end
		--Diminish fuze length on impact
		if self.TravelImpulse:MagnitudeIsGreaterThan(1) then
			self.fuzeDelay = self.fuzeDelay - self.TravelImpulse.Magnitude * self.fuzeDecreaseIncrement;
		end
	elseif self:IsActivated() then
		self.fuze = Timer();
	end
end