function Create(self)
	self.origActivationDelay = self.ActivationDelay;
	self.spinDownTimer = Timer();
	self.currentlySpinningDown = false;
end

function ThreadedUpdate(self)
	if not self.currentlySpinningDown and not self:IsActivated() and not self:IsReloading() and self.ActiveSound:IsBeingPlayed() then
		self.spinDownTimer:Reset();
		self.currentlySpinningDown = true;
		self.activationDelay = self.origActivationDelay;
	elseif (self.currentlySpinningDown and self.spinDownTimer:IsPastSimMS(self.DeactivationDelay)) or self:IsReloading() then
		self.ActivationDelay = self.origActivationDelay;
		self.currentlySpinningDown = false;
	elseif self.currentlySpinningDown and self:IsActivated() then
		self.ActivationDelay = self.origActivationDelay * self.spinDownTimer.ElapsedSimTimeMS / self.DeactivationDelay;
		self.currentlySpinningDown = false;
	end
end