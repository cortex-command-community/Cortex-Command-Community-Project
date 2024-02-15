function Create(self)
	self.impactSound = CreateSoundContainer("Flesh Giblet Impact", "Base.rte");	
end

function OnCollideWithTerrain(self, terrainID)
	if not self.Impacted then
		self.impactSound:Play(self.Pos);
		self.Impacted = true;
	end
end