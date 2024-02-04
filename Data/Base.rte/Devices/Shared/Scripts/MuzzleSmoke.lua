function Create(self)

	if self.Magazine then
		local particleCount = self.Magazine.NextRound.ParticleCount;
		local particleMass = self.Magazine.NextRound.NextParticle.Mass;
		local fireVel = self.Magazine.NextRound.FireVel;
	
		self.muzzleSmokeSize = math.min(50, particleCount * particleMass * fireVel);
	end
	
	self.muzzleSmokeSize = self:NumberValueExists("MuzzleSmokeSize") and self:GetNumberValue("MuzzleSmokeSize") or self.muzzleSmokeSize;
	if self.muzzleSmokeSize == nil then
		self.muzzleSmokeSize = self.Mass/1.5;
	end
	
	self.muzzleSmokeVel = math.sqrt(self.muzzleSmokeSize) * 5;
	
	self.fireSmokeEffect = {};
	self.fireSmokeEffect.Position = self.MuzzlePos;
	self.fireSmokeEffect.Source = self;
	self.fireSmokeEffect.RadAngle = self.RotAngle;
	self.fireSmokeEffect.Power = self.muzzleSmokeSize;
	self.fireSmokeEffect.Spread = (self.ShakeRange + self.SharpShakeRange) * 0.5 + self.ParticleSpreadRange;
	self.fireSmokeEffect.SmokeMult = 0.5;
	self.fireSmokeEffect.ExploMult = 1;
	self.fireSmokeEffect.WidthSpread = 0;
	self.fireSmokeEffect.VelocityMult = 0.5;
	self.fireSmokeEffect.LingerMult = 0.45;
	self.fireSmokeEffect.AirResistanceMult = 1;
	self.fireSmokeEffect.GravMult = 1;
	
	self.particleUtility = require("Scripts/Utility/ParticleUtility");	
	
end

function OnFire(self)

	local flip = self.HFlipped and math.pi or 0;
	local angle = self.RotAngle + flip;
	self.fireSmokeEffect.Position = self.MuzzlePos;
	self.fireSmokeEffect.RadAngle = angle;
	
	self.particleUtility:CreateDirectionalSmokeEffect(self.fireSmokeEffect);

end