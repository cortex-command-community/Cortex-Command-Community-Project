function Create(self)
	self.healAmount = 1;
	self.regenDelay = 750;
	self.regenTimer = Timer();

	self.lastWoundCount = self.WoundCount;
end

function ThreadedUpdate(self)
	if self.regenTimer:IsPastSimMS(self.regenDelay) then
		self.regenTimer:Reset();
		if self.Health > 0 then
			local damageRatio = (self.WoundCount - self.lastWoundCount)/self:GetGibWoundLimit(true, false, false) + (self.PrevHealth - self.Health)/self.MaxHealth;
			if damageRatio > 0 then
				self.regenDelay = self.regenDelay * (1 + damageRatio);
			else
				local healed = self:RemoveWounds(1);
				if healed ~= 0 and self.Health < self.MaxHealth then
					self:AddHealth(self.healAmount);
					if self.Health > self.MaxHealth	then
						self.Health = self.MaxHealth;
					end
				end
			end
		end
		self.lastWoundCount = self.WoundCount;
	end
end