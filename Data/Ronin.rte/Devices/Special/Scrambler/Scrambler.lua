function Create(self)
	self.fuzeDelay = 3500;
	self.fuzeDecreaseIncrement = 15;
end

function Update(self)
	if self.fuze then
		if self.fuze:IsPastSimMS(self.fuzeDelay) then
			local payload = CreateMOSRotating("Ronin Scrambler Payload", "Ronin.rte");
			if payload then
				payload.Pos = self.Pos;
				MovableMan:AddParticle(payload);
				payload:GibThis();
			end
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