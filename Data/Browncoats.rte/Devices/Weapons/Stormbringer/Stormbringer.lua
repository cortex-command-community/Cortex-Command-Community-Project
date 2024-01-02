function OnFire(self)
	
	self.mechSound:Play(self.Pos);
	
end

function Create(self)

	self.mechSound = CreateSoundContainer("Mech Browncoat A", "Browncoat.rte");
	self.mechSound.Volume = 0.65

end