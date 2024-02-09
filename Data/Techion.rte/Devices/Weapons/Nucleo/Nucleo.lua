function Create(self)
	self.spread = math.rad(self.ParticleSpreadRange);
	
	self.bulletTable = {};
end

function Update(self)
	if self.FiredFrame then
		local bullet = CreateMOSRotating("Particle Nucleo");
		bullet.Pos = self.MuzzlePos;
		bullet.Vel = self.Vel + Vector(30 * self.FlipFactor, 0):RadRotate(self.RotAngle + self.spread * RangeRand(-0.5, 0.5));
		bullet.Team = self.Team;
		MovableMan:AddParticle(bullet);
		bullet:SendMessage("Nucleo_ConnectableParticles", self.bulletTable);
		table.insert(self.bulletTable, bullet.UniqueID);
	end
	
	if self:DoneReloading() then
		self.bulletTable = {};
	end
end