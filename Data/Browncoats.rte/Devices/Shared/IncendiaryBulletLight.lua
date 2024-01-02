function Create(self)
	self.trailGlow = CreateMOPixel("Incendiary Bullet Trail Glow Light");
	self.trailLength = 40;

	self.trailGlow.Pos = self.Pos - Vector(self.Vel.X, self.Vel.Y) * rte.PxTravelledPerFrame * 0.5;
	MovableMan:AddParticle(self.trailGlow);

	self.fire = CreatePEmitter("Flame Hurt Short");
end

function Update(self)
	if self.trailGlow and MovableMan:IsParticle(self.trailGlow) then
		self.trailGlow.Pos = self.Pos - Vector(self.Vel.X, self.Vel.Y):SetMagnitude(math.min(self.Vel.Magnitude * rte.PxTravelledPerFrame, self.trailLength) * 0.5);
		self.trailGlow.Vel = self.Vel * 0.5;
		self.trailGlow.Lifetime = self.Age + TimerMan.DeltaTimeMS;
	end

	if math.random() < 0.5 then
		local particle = CreateMOPixel("Ground Fire Burn Particle");
		particle.Pos = Vector(self.Pos.X, self.Pos.Y) - Vector(self.Vel.X, self.Vel.Y) * rte.PxTravelledPerFrame * math.random();
		particle.Vel = self.Vel * RangeRand(0.5, 1.0) + Vector(math.random(10), 0):RadRotate(math.random() * math.pi * 2);
		particle.Lifetime = particle.Lifetime * RangeRand(0.5, 1.0);
		particle.Sharpness = particle.Sharpness * RangeRand(0.1, 1.0);
		particle.Team = self.Team;
		particle.IgnoresTeamHits = true;
		MovableMan:AddParticle(particle);

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
			else
				local smoke = CreateMOSParticle("Tiny Smoke Ball 1");
				smoke.Pos = Vector(self.Pos.X, self.Pos.Y);
				smoke.Vel = Vector(self.Vel.X, self.Vel.Y) * 0.5;
				smoke.Lifetime = math.random(200, 400);
				MovableMan:AddParticle(smoke);
			end
		end
	end
end