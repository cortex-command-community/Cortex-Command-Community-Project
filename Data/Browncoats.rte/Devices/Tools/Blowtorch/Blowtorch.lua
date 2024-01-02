function Create(self)
	self.startSound = CreateSoundContainer("Ignite DG-1000", "Browncoats.rte");
	self.endSound = CreateSoundContainer("Loop End DG-1000", "Browncoats.rte");
end

function ThreadedUpdate(self)
	if self:IsActivated() and self.RoundInMagCount ~= 0 then
		if not self.triggerPulled then
			self.startSound:Play(self.MuzzlePos);
		end
		self.triggerPulled = true;
	else
		if self.triggerPulled then
			self.endSound:Play(self.MuzzlePos);
		end
		self.triggerPulled = false;
	end
end