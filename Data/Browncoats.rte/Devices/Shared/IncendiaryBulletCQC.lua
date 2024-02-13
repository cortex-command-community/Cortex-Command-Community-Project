function Create(self)
	self.fire = CreatePEmitter("Flame Hurt Short");
	self.smokeTwirlCounter = math.random() < 0.5 and math.pi or 0;	

	self.deviation = 10;
	
	self.deviationTimer = Timer();
	self.deviationDelayMin = 13;
	self.deviationDelayMax = 200;
	self.deviationDelay = 75;
	
	self.smokeTrailLifeTime = 5000;
	self.smokeTrailTwirl = 0.0;
	
	self.smokeTwirlCounter = math.random() < 0.5 and math.pi or 0;
	
	self.trailAcc = 0;
end

function Update(self)
	local velFactor = math.floor(1 + math.sqrt(self.Vel.Magnitude)/(1 + self.Age * 0.01));

	local particle = CreateMOPixel("Fire Burn Particle");
	particle.Pos = Vector(self.Pos.X, self.Pos.Y) - Vector(self.Vel.X, self.Vel.Y) * rte.PxTravelledPerFrame * 0.5;
	particle.Vel = self.Vel * 0.5 + Vector(math.random(5, 10)/velFactor, 0):RadRotate(math.random() * math.pi * 2);
	particle.Lifetime = particle.Lifetime * RangeRand(0.3, 0.5);
	particle.GlobalAccScalar = -math.random();
	particle.Team = self.Team;
	particle.IgnoresTeamHits = true;
	MovableMan:AddParticle(particle);

	if self.deviationTimer:IsPastSimMS(self.deviationDelay) then
		self.deviationTimer:Reset()
		self.deviationDelay = RangeRand(self.deviationDelayMin, self.deviationDelayMax)
		self.Vel = Vector(self.Vel.X, self.Vel.Y):DegRotate(self.deviation * RangeRand(-0.5, 0.5))
	end
	
	local offset = self.Vel * rte.PxTravelledPerFrame;
	local trailLength = math.floor(offset.Magnitude * 0.3 - 1);
	for i = 1, trailLength do
		local effect = CreateMOSParticle("Flame Smoke 1 Micro", "Base.rte");
		effect.Lifetime = effect.Lifetime * RangeRand(0.5, 1);

		effect.AirResistance = effect.AirResistance * RangeRand(0.9, 1);
		effect.GlobalAccScalar = effect.GlobalAccScalar * math.random();

		effect.Pos = self.Pos - offset + (offset * i/trailLength);
		effect.Vel = self.Vel * 0.1 + Vector(1, math.sin(self.smokeTwirlCounter) + RangeRand(-0.1, 0.1)):RadRotate(self.Vel.AbsRadAngle);

		self.smokeTwirlCounter = self.smokeTwirlCounter + RangeRand(-0.2, 0.4);
		MovableMan:AddParticle(effect);
	end
	
	if self.ToDelete then
		if self.Age < self.Lifetime then
			local hitPos = Vector(self.Pos.X, self.Pos.Y);
			local trace = Vector(self.Vel.X, self.Vel.Y) * rte.PxTravelledPerFrame;
			local skipPx = 2;
			local obstacleRay = SceneMan:CastObstacleRay(Vector(self.Pos.X, self.Pos.Y), trace, Vector(), hitPos, rte.NoMOID, self.Team, rte.airID, skipPx);
			if obstacleRay >= 0 then
				self.fire.Pos = hitPos;
				self.fire.Vel = Vector(self.Vel.X, self.Vel.Y):SetMagnitude(skipPx);
				MovableMan:AddParticle(self.fire);
			end
		end
	end	
end