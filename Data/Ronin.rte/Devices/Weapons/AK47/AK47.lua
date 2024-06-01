function OnFire(self)
	-- Higher pitch and volume as you approach empty
	self.mechSound.Pitch = 1;
	self.mechSound.Volume = 1;

	local parent = self:GetRootParent();
	if IsActor(parent) and ToActor(parent):IsPlayerControlled() then
		if self.RoundInMagCount < 11 then
			self.mechSound.Pitch = 1.1 + (11 - self.RoundInMagCount)/100;
			self.mechSound.Volume = 1.3 + (11 - self.RoundInMagCount)/25;
		end
	end
	
	self.mechSound:Play(self.Pos);
	
	if self.RoundInMagCount == 0 then
		self.lastRoundSound:Play(self.Pos);
	end
end

function Create(self)
	self.mechSound = CreateSoundContainer("Mech Ronin AK-47", "Ronin.rte");
	self.lastRoundSound = CreateSoundContainer("Last Round Ronin", "Ronin.rte");
	self.lastRoundSound.Pitch = 0.75;
end