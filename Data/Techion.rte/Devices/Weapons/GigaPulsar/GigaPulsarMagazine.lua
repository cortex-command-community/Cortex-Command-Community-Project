function Create(self)
	self.smokeTimer = Timer();
	self.smokeDelay = 25;
end

function Update(self)
	if self.Sharpness == 1 and self.smokeTimer:IsPastSimMS(self.smokeDelay) then
		self.smokeTimer:Reset();
		self.smokeDelay = self.smokeDelay * (1 + self.RoundCount/self.Capacity) + 1;

		local smoke = CreateMOSParticle("Tiny Smoke Ball 1");
		smoke.Pos = self.Pos + Vector(math.random(-1, 1), math.random(-1, 1));
		smoke.Vel = self.Vel + Vector(RangeRand(-2, 2), RangeRand(-2, 2));
		smoke.Lifetime = smoke.Lifetime * RangeRand(0.5, 1.0);
		MovableMan:AddParticle(smoke);
	end
end