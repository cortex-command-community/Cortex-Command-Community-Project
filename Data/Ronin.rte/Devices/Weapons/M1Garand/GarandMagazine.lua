function Create(self)
	self.pingSound = CreateSoundContainer("Ronin M1 Garand Ping", "Ronin.rte");
end

function OnDetach(self, exParent)
	if self.RoundCount == 0 and exParent then
		self.Vel = self.Vel + Vector(0, -7):RadRotate(exParent.RotAngle);
		self.pingSound:Play(self.Pos);
	end
end