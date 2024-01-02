function Create(self)
	self.spread = math.rad(self.ParticleSpreadRange);
end
function OnFire(self)
	local bullet = CreateMOSRotating("Particle Nucleo");
	bullet.Pos = self.MuzzlePos;
	bullet.Vel = self.Vel + Vector(30 * self.FlipFactor, 0):RadRotate(self.RotAngle + self.spread * RangeRand(-0.5, 0.5));
	bullet.Sharpness = self.UniqueID;
	bullet.Team = self.Team;
	MovableMan:AddParticle(bullet);
end