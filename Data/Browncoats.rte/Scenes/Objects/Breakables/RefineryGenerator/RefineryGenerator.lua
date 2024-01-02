function Create(self)

	self.Activity = ToGameActivity(ActivityMan:GetActivity());
	
end

function OnDestroy(self)

	self.Activity:SendMessage("RefineryAssault_RefineryGeneratorBroken");

end