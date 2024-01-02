function Create(self)
	self.shotsPerBurst = self:NumberValueExists("ShotsPerBurst") and self:GetNumberValue("ShotsPerBurst") or 3;
	self.coolDownDelay = (60000/self.RateOfFire) + 100;
end

function ThreadedUpdate(self)
	if self.Magazine then
		if self.coolDownTimer then
			if self.coolDownTimer:IsPastSimMS(self.coolDownDelay) and not (self:IsActivated() and self.triggerPulled) then
				self.coolDownTimer, self.shotCounter = nil;
			else
				self:Deactivate();
				local parent = self:GetRootParent();
				if parent and IsActor(parent) and not ToActor(parent):IsPlayerControlled() then
					self.triggerPulled = false;
				end
			end
		elseif self.shotCounter then
			self.triggerPulled = self:IsActivated();
			self:Activate();
		end
	else
		self.coolDownTimer, self.shotCounter = nil;
	end
end

function OnFire(self)
	if self.shotCounter then
		self.shotCounter = self.shotCounter + 1;
		if self.shotCounter >= self.shotsPerBurst then
			self.coolDownTimer = Timer();
		end
	else
		self.shotCounter = 1;
	end
end