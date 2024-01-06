function OnFire(self)

	self.mechSound:Play(self.Pos);
	local flip = self.HFlipped and math.pi or 0;
	local angle = self.RotAngle + flip;
	self.fireSmokeEffect.Position = self.MuzzlePos;
	self.fireSmokeEffect.RadAngle = angle;

	self.utilityHandler:CreateDirectionalSmokeEffect(self.fireSmokeEffect);

end

function Create(self)

	self.mechSound = CreateSoundContainer("Mech Browncoat HG-10", "Browncoats.rte");
	
	self.fireSmokeEffect = {};
	self.fireSmokeEffect.Position = self.Pos;
	self.fireSmokeEffect.Source = self;
	self.fireSmokeEffect.RadAngle = self.RotAngle;
	self.fireSmokeEffect.Power = 15;
	self.fireSmokeEffect.Spread = 15;
	self.fireSmokeEffect.SmokeMult = 1;
	self.fireSmokeEffect.ExploMult = 1;
	self.fireSmokeEffect.WidthSpread = 2;
	self.fireSmokeEffect.VelocityMult = 1.5;
	self.fireSmokeEffect.LingerMult = 0.5;
	self.fireSmokeEffect.AirResistanceMult = 1;
	self.fireSmokeEffect.GravMult = 0.5;
	
	self.utilityHandler = require("Activities/Utility/UtilityHandler");
	self.utilityHandler:Initialize(false);
	
end