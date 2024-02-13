function Create(self)
	self.impulseSoundThreshold = 200;
	self.impulseSound = CreateSoundContainer("Sandbag Thud", "Ronin.rte");

	self.width = ToMOSprite(self):GetSpriteWidth();
	self.height = ToMOSprite(self):GetSpriteHeight();
end

function Update(self)
	if self.ClassName == "ThrownDevice" and not self:GetParent() then
		local part = CreateMOSRotating(self:GetModuleAndPresetName());
		part.Pos = self.Pos;
		part.HFlipped = self.HFlipped;
		part.Vel = self.Vel;
		MovableMan:AddParticle(part);

		self.ToDelete = true;
	elseif self.TravelImpulse:MagnitudeIsGreaterThan(self.Mass) then
		for i = 1, self.TravelImpulse.Magnitude * 0.3 do
			if self.Mass > 5 then
				local particle = CreateMOPixel("Sandbag Particle ".. math.random(5));
				local vector = Vector(self.width * RangeRand(-0.4, 0.4), self.height * RangeRand(-0.4, 0.4)):RadRotate(self.RotAngle);
				particle.Pos = self.Pos + vector;
				particle.Vel = vector:SetMagnitude(math.random() * self.Vel.Magnitude + 1);
				MovableMan:AddParticle(particle);

				self.Mass = self.Mass - particle.Mass;
			else
				break;
			end
		end

		if self.TravelImpulse:MagnitudeIsGreaterThan(self.impulseSoundThreshold) then
			self.impulseSound:Play(self.Pos);
		end
		
		self.GibWoundLimit = self.Mass * 3;
		self.GibImpulseLimit = self.Mass * 30;
	end
end