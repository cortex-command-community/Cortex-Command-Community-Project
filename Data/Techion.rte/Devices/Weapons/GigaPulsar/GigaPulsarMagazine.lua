function Create(self)
	self.smokeTimer = Timer();
	self.smokeDelay = 25;
end

function ThreadedUpdate(self)
	if self.Sharpness == 1 and self.smokeTimer:IsPastSimMS(self.smokeDelay) then
		self.smokeTimer:Reset();
		self.smokeDelay = self.smokeDelay * (1 + self.RoundCount/self.Capacity) + 1;

		self.smoke = CreateMOSParticle("Tiny Smoke Ball 1");
		self.smoke.Pos = self.Pos + Vector(math.random(-1, 1), math.random(-1, 1));
		self.smoke.Vel = self.Vel + Vector(RangeRand(-2, 2), RangeRand(-2, 2));
		self.smoke.Lifetime = self.smoke.Lifetime * RangeRand(0.5, 1.0);
		self:RequestSyncedUpdate();
	end
end

function SyncedUpdate(self)
	if self.smoke then
		MovableMan:AddParticle(self.smoke);
		self.smoke = nil;
	end
end