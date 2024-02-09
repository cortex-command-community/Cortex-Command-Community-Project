function Create(self)
	self.AirResistance = self.AirResistance * RangeRand(1.0, 1.5);
	self.trailLength = 30;

	self.trailPar = {};
	self.trailParCount = 4;
	for i = 1, self.trailParCount do
		local particle = CreateMOPixel("Dummy Repeater Trail Glow");
		particle.Pos = self.Pos;
		particle.Sharpness = particle.Sharpness/i;

		particle.Team = self.Team;
		particle.IgnoresTeamHits = true;

		particle.Lifetime = 100;
		MovableMan:AddParticle(particle);
		table.insert(self.trailPar, particle);
	end

	self.endPar = CreateMOSParticle("Tiny Smoke Ball 1 Glow Yellow");
end

function Update(self)
	if not self.ToDelete then
		for i = 1, self.trailParCount do
			if self.trailPar[i] and MovableMan:IsParticle(self.trailPar[i]) then
				self.trailPar[i].Pos = self.Pos + Vector(RangeRand(-0.5, 0.5), RangeRand(-0.5, 0.5)) - Vector(self.PrevVel.X, self.PrevVel.Y):SetMagnitude(math.min(self.PrevVel.Magnitude, self.trailLength + 1) * i/self.trailParCount);
				self.trailPar[i].Vel = Vector(self.PrevVel.X, self.PrevVel.Y);
				self.trailPar[i].Lifetime = self.Age + math.random(20, 40);
			end
		end
	end
	
	if self.Vel:MagnitudeIsLessThan(4) then
		self.ToDelete = true;
	end
end

function Destroy(self)
	self.endPar.Pos = Vector(self.Pos.X, self.Pos.Y) + Vector(self.Vel.X, self.Vel.Y) * 0.16;
	self.endPar.Vel = Vector(self.Vel.X + math.random(-5, 5), self.Vel.Y + math.random(-5, 5)) * 0.5;
	self.endPar.HitsMOs = true;
	self.endPar.Mass = self.Mass;
	self.endPar.Lifetime = self.endPar.Lifetime/(1 + self.Age/self.Lifetime);
	MovableMan:AddParticle(self.endPar);
end