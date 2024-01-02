function Create(self)
	self.shellMOSParticle = self:StringValueExists("CylinderShellMOSParticle") and self:GetStringValue("CylinderShellMOSParticle") or "Base.rte/Casing";
	self.shellMOSRotating = self:StringValueExists("CylinderShellMOSRotating") and self:GetStringValue("CylinderShellMOSRotating") or nil;
end

function OnDetach(self, exParent)
	local shellsToEject = self.Capacity - self.RoundCount;
	for i = 1, shellsToEject do
		local shell = self.shellMOSRotating and CreateMOSRotating(self.shellMOSRotating) or CreateMOSParticle(self.shellMOSParticle);
		shell.Pos = exParent.Pos;
		shell.Vel = exParent.Vel + Vector(RangeRand(-3, 0) * exParent.FlipFactor, 0):RadRotate(exParent.RotAngle + RangeRand(-0.3, 0.3));
		shell.AngularVel = RangeRand(-1, 1);
		MovableMan:AddMO(shell);
	end
	self.ToDelete = true;
end