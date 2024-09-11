function Update(self)
	if self.Age > self.Lifetime - 100 then
		self:GibThis();
		local igniter = CreateMOSRotating("Browncoat Boss Oil Bomb Igniter", "Browncoats.rte");
		igniter:SetNumberValue("Secondary", 1);
		igniter.Pos = self.Pos
		igniter.HFlipped = self.HFlipped;
		igniter.Vel = self.Vel
		igniter.AngularVel = self.AngularVel
		igniter.RotAngle = self.RotAngle
		igniter.Team = self.Team;
		MovableMan:AddParticle(igniter);
	end
end