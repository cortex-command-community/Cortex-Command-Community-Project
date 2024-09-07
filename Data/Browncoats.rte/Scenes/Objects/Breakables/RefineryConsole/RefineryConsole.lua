function Create(self)

	self.Activity = ToGameActivity(ActivityMan:GetActivity());
	
	self.loopSound = CreateSoundContainer("Yskely Refinery Refinery Console Loop", "Browncoats.rte");
	self.loopSound:Play(self.Pos);
end

function OnDestroy(self)

	self.Activity:SendMessage("RefineryAssault_RefineryConsoleBroken");

end