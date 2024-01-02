function OnFire(self)
	
	self.mechSound:Play(self.Pos);
	
end

function Create(self)

	self.mechSound = CreateSoundContainer("Mech Browncoat PY-7", "Browncoat.rte");

end