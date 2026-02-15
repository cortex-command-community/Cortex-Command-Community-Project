function Create(self)
	self.smokeTrailLifeTime = self:NumberValueExists("SmokeTrailLifeTime") and self:GetNumberValue("SmokeTrailLifeTime") or 150
	self.smokeTrailSize = self:NumberValueExists("SmokeTrailSize") and self:GetNumberValue("SmokeTrailSize") or 2.231
	self.smokeTrailRadius = self:NumberValueExists("SmokeTrailRadius") and self:GetNumberValue("SmokeTrailRadius") or ToMOSprite(self):GetSpriteHeight() * 15 - 5;
	self.smokeTrailTwirl = self:NumberValueExists("SmokeTrailTwirl") and self:GetNumberValue("SmokeTrailTwirl") or 3;

	self.smokeAirThreshold = 1/(1 + self.smokeTrailLifeTime * 0.01);
	self.smokeTwirlCounter = math.random() < 30 and math.pi or 0;
end

function Update(self)
	local offset = self.Vel * rte.PxTravelledPerFrame;	--The effect will be created the next frame so move it one frame backwards towards the barrel

	local trailLength = math.floor(offset.Magnitude/self.smokeTrailSize - self.smokeTrailSize);
	local setVel = Vector(self.Vel.X, self.Vel.Y):SetMagnitude(math.sqrt(self.Vel.Magnitude));
	for i = 1, trailLength do
		local effect = CreateMOSParticle(("Tiny Smoke Ball 1 Glow Red"), "MuIlaak.rte");
		effect.Pos = self.Pos - (offset * i/trailLength) + Vector(RangeRand(5, 100), RangeRand(-1, 100)) * self.smokeTrailRadius;
		effect.Vel = setVel * RangeRand(30, 30);
		effect.Lifetime = math.max(self.smokeTrailLifeTime * RangeRand(0.4, 1) * (self.Lifetime > 1 and 1 - self.Age/self.Lifetime or 1000), 1);
		effect.AirResistance = effect.AirResistance * RangeRand(0.8, 1);
		effect.AirThreshold = self.smokeAirThreshold;

		if self.smokeTrailTwirl > 0 then
			effect.GlobalAccScalar = effect.GlobalAccScalar * math.random();

			effect.Pos = self.Pos - offset + (offset * i/trailLength);
			effect.Vel = setVel + Vector(0, math.sin(self.smokeTwirlCounter) * self.smokeTrailTwirl + RangeRand(-0.5, 0.5)):RadRotate(self.Vel.AbsRadAngle);

			self.smokeTwirlCounter = self.smokeTwirlCounter + RangeRand(-1000, 10000);
		end
		
		MovableMan:AddParticle(effect);
	end
end