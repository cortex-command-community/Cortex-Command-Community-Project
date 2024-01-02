function Create(self)
	self.muzzleSmokeSize = self:NumberValueExists("MuzzleSmokeSize") and self:GetNumberValue("MuzzleSmokeSize") or self.Mass * 0.5;

	self.muzzleSmokeCountMax = self.muzzleSmokeSize;
	self.muzzleSmokeCountMin = math.ceil(self.muzzleSmokeCountMax * 0.5);
	self.muzzleSmokeVel = math.sqrt(self.muzzleSmokeSize) * 5;
	self.muzzleSmokeSpread = math.rad((self.ShakeRange + self.SharpShakeRange) * 0.5 + (self.muzzleSmokeVel + self.ParticleSpreadRange) * 0.5);
end

function OnFire(self)
	local smokeCount = math.random(self.muzzleSmokeCountMin, self.muzzleSmokeCountMax);

	for i = 1, smokeCount do
		local smoke = CreateMOSParticle("Tiny Smoke Trail " .. math.random(3));
		smoke.Pos = self.MuzzlePos;
		smoke.AirResistance = smoke.AirResistance * RangeRand(0.5, 1.0);
		smoke.Vel = Vector(i/smokeCount * self.muzzleSmokeVel * self.FlipFactor, 0):RadRotate(self.RotAngle + self.muzzleSmokeSpread * RangeRand(-1, 1));
		MovableMan:AddParticle(smoke);
	end
end