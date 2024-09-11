function Create(self)
	self.glow = CreateMOPixel("Shielder Wall Glow", "Dummy.rte");
	self.glow.Pos = self.Pos;
	self.glow.EffectRotAngle = self.RotAngle;
	MovableMan:AddParticle(self.glow);
	self.glowID = self.glow.UniqueID;

	self.AngularVel = 0;
	self.baseLifetime = self.Lifetime;
end

function Update(self)
	if self.glow and self.glow.UniqueID == self.glowID then
		self.glow.Pos = self.Pos;
		if self.AngularVel ~= 0 then
			self.glow.EffectRotAngle = self.RotAngle;
		end
		--To-do: add flicker
	else
		self.glow = nil;
	end

	self.AngularVel = 0;

	if self.PinStrength == 0 and self.Vel:MagnitudeIsLessThan(1) then
		self.PinStrength = self.Mass;
	else
		self.Vel = Vector();
	end
	
	if self.GibWoundLimit > 0 then
		self.Lifetime = math.max(self.baseLifetime * (1 - self.WoundCount/self.GibWoundLimit), 1);
	end
end

function Destroy(self)
	if self.glow then
		self.glow.ToDelete = true;
	end
end