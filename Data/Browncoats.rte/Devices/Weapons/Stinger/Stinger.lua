function OnFire(self)

	self.mechSound:Play(self.Pos);

end

function Create(self)

	self.mechSound = CreateSoundContainer("Mech Browncoat HG-10", "Browncoats.rte");
	
	
end