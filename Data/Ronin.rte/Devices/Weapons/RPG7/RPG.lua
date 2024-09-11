function Create(self)
	self.impulseLimit = self.GibImpulseLimit;
	self.GibImpulseLimit = self.GibImpulseLimit * 3;
end

function Update(self)
	local orientation = math.min(math.max(math.cos(self.PrevRotAngle + (self.HFlipped and math.pi or 0) - self.PrevVel.AbsRadAngle), 0.1), 0.9);
	self.GlobalAccScalar = 1 - orientation/(1 + math.abs(self.AngularVel));
	local impulseLimit = self.impulseLimit * (1 - orientation);
	if self.TravelImpulse:MagnitudeIsGreaterThan(impulseLimit) then
		self.RotAngle = self.PrevRotAngle;
		self:GibThis();
	end
end