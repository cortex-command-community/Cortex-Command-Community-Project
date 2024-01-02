function OnFire(self)

	self.mechSound:Play(self.Pos);

end

function Create(self
)
	self.mechSound = CreateSoundContainer("Mech Browncoat IN-2", "Browncoats.rte");
	
end