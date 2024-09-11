function Create(self)
	self.origMass = self.Mass;
	self.explosion = CreateMOSRotating("Molotov Cocktail Explosion", "Ronin.rte");
end

function Update(self)
	if self.ID == self.RootID then
		if self.thrown == false then
			self.AngularVel = self.AngularVel - self.Vel.Magnitude * self.FlipFactor * math.random();
			self.thrown = true;
		end
		self.Mass = self.origMass + math.sqrt(self.PrevVel.Magnitude);
	else
		self.thrown = false;
		self.Mass = self.origMass;
	end

	if not self.lit and self.WoundCount > 0 then
		--Slight chance for projectiles to light the molotov instead of breaking it
		if math.random(0, 10) < self.WoundCount then
			self:Activate();
		else
			self:GibThis();
		end
	end
	
	self.lit = self:IsActivated();
end

function Destroy(self)
	--Explode into flames only if lit
	if self.lit then
		self.explosion.Pos = Vector(self.Pos.X, self.Pos.Y);
		self.explosion.Vel = Vector(self.Vel.X, self.Vel.Y);
		MovableMan:AddParticle(self.explosion);
		self.explosion:GibThis();
	end
end