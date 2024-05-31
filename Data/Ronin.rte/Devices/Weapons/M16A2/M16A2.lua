function OnFire(self)
	-- Higher pitch and volume as you approach empty
	self.mechSound.Pitch = 1;
	self.mechSound.Volume = 1;

	local parent = self:GetRootParent();
	if IsActor(parent) and ToActor(parent):IsPlayerControlled() then
		if self.RoundInMagCount < 11 then
			self.mechSound.Pitch = 1.2 + (11 - self.RoundInMagCount)/100;
			self.mechSound.Volume = 1.3 + (11 - self.RoundInMagCount)/25;
		end
	end
	
	self.mechSound:Play(self.Pos);
end

function Create(self)
	self.mechSound = CreateSoundContainer("Mech Ronin M16A2", "Ronin.rte");
end