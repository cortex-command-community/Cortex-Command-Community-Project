function Create(self)
	self.smoker = CreateAEmitter("Laser Rifle Magazine Smoker", "Techion.rte");
end

function OnDetach(self, exParent)
	self.smoker.Throttle = -(self.RoundCount/self.Capacity);
	self:AddAttachable(self.smoker);
end