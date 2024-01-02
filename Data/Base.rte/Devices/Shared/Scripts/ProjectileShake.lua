function Create(self)
	self.projectileShake = self:NumberValueExists("ProjectileShake") and self:GetNumberValue("ProjectileShake") or 1;
end

function Update(self)
	self.Vel = Vector(self.Vel.X, self.Vel.Y):DegRotate(self.projectileShake * 0.5 - self.projectileShake * math.random());
end