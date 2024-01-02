function Create(self)
	self.chainCount = 24;
	self.fireVel = 50;
	self.twirlCounter = math.random() * math.pi * 2;

	for i = 1, self.chainCount do
		local chain = CreateMOSParticle("Particle Mauler Chainshot ".. i % 2 + 1);
		local ratio = i/self.chainCount;
		local vector = Vector(0, math.sin(self.twirlCounter)):RadRotate(self.RotAngle);
		chain.Pos = self.Pos + vector;
		chain.Vel = self.Vel + Vector(self.fireVel * (1 - 0.25 * ratio) * self.FlipFactor, 0):RadRotate(self.RotAngle) + vector;
		chain.Sharpness = chain.Sharpness * (1 - 0.5 * ratio);
		chain.Lifetime = chain.Lifetime * RangeRand(0.7, 1.3);
		chain.AngularVel = RangeRand(0, 5);
		chain.Team = self.Team;
		chain.IgnoresTeamHits = true;

		MovableMan:AddParticle(chain);

		self.twirlCounter = self.twirlCounter + RangeRand(0, 0.5);
	end
end