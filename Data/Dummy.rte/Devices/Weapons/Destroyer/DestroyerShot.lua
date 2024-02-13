function Create(self)
	self.lifeTimer = Timer();
	self.lifeTimer:SetSimTimeLimitMS(self.Lifetime - 100);
	self.emitTimer = Timer();

	self.startSpeed = self.Vel.Magnitude;
	self.acceleration = 0.1;
end

function Update(self)
	self.Vel = self.Vel:MagnitudeIsLessThan(self.startSpeed) and Vector(self.Vel.X, self.Vel.Y):SetMagnitude(self.Vel.Magnitude + self.acceleration) or self.Vel;

	if self.emitTimer:IsPastSimMS(6) then
		self.emitTimer:Reset();
		local particleCount = 1 + math.sqrt(self.Vel.Magnitude * 0.2);
		for i = 1, particleCount do
			local damagePar = CreateMOPixel("Dummy.rte/Destroyer Emission Particle ".. math.random(2));
			damagePar.Pos = self.Pos;
			damagePar.Vel = self.Vel * 0.5 + Vector(particleCount + math.random(10), 0):RadRotate(math.pi * 2 * math.random());
			damagePar.Team = self.Team;
			damagePar.IgnoresTeamHits = true;
			MovableMan:AddParticle(damagePar);
		end
	end
	
	if self.lifeTimer:IsPastSimTimeLimit() then
		self:GibThis();
	end
end