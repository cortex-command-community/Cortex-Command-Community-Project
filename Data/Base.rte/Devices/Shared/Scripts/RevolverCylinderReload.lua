function Create(self)
	self.shellsToEject = self.RoundInMagCount;
	self.shellMOSParticle = self:StringValueExists("CylinderShellMOSParticle") and self:GetStringValue("CylinderShellMOSParticle") or "Base.rte/Casing";
	self.shellMOSRotating = self:StringValueExists("CylinderShellMOSRotating") and self:GetStringValue("CylinderShellMOSRotating") or nil;
end

function Update(self)
	if self.Magazine then
		self.shellsToEject = self.Magazine.Capacity - self.Magazine.RoundCount;
	elseif self.shellsToEject > 0 then
		for i = 1, self.shellsToEject do
			local shell = self.shellMOSRotating and CreateMOSRotating(self.shellMOSRotating) or CreateMOSParticle(self.shellMOSParticle);
			shell.Pos = self.Pos;
			shell.Vel = self.Vel + Vector(RangeRand(-3, 0) * self.FlipFactor, 0):RadRotate(self.RotAngle + RangeRand(-0.3, 0.3));
			shell.AngularVel = RangeRand(-1, 1);
			MovableMan:AddParticle(shell);
		end
		
		self.shellsToEject = 0;
	end
end