function OnFire(self)
	if self.RoundInMagCount == 0 then
		self.lastRoundSound:Play(self.Pos);
		self.Empty = true;
	else
		self.mechSound:Play(self.Pos);
	end
end

function OnReload(self)
	if not self.Empty then
		self.fullReloadStartSound:Play(self.Pos);
	end
	self.Empty = false;
end

function Create(self)
	self.mechSound = CreateSoundContainer("Mech Ronin M1 Garand", "Ronin.rte");
	self.lastRoundSound = CreateSoundContainer("Last Round Ronin M1 Garand", "Ronin.rte");
	self.fullReloadStartSound = CreateSoundContainer("Full Reload Start Ronin M1 Garand", "Ronin.rte");
end