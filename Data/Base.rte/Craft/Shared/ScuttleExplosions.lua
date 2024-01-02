function Create(self)
	self.explosionTimer = Timer();

	self.height = ToMOSprite(self):GetSpriteHeight();
	self.width = ToMOSprite(self):GetSpriteWidth();

	self.explosionDelay = self:NumberValueExists("ScuttleExplosionDelay") and self:GetNumberValue("ScuttleExplosionDelay") or 5000/math.sqrt(self.width + self.height);
end

function Update(self)
	if self.Status == Actor.DYING and self.explosionTimer:IsPastSimMS(self.explosionDelay) then
		self.explosionTimer:Reset();
		local explosion = CreateAEmitter("Scuttle Explosion", "Base.rte");
		explosion.Pos = self.Pos + Vector(self.width * 0.5 * RangeRand(-0.9, 0.9), self.height * 0.5 * RangeRand(-0.9, 0.9)):RadRotate(self.RotAngle);
		explosion.Vel = self.Vel;
		MovableMan:AddParticle(explosion);
	end
end