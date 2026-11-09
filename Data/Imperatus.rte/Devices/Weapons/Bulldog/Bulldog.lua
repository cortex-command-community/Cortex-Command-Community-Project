function Create(self)
	self.origActivationDelay = self.ActivationDelay;
	self.spinDownTimer = Timer();
	self.currentlySpinningDown = false;
	self.wasActivated = false;

	self.maxRateOfFire = self.RateOfFire;
	self.minRateOfFire = self.maxRateOfFire * 0.5;
	self.RateOfFire = self.minRateOfFire;
	self.increasePerShot = 1.05;
	self.decreasePerFrame = 0.99;
end

function Update(self)
	-- The gun starts spinning down when it is let go of after having been activated. It keeps track of that by itself instead of going by whether its ActiveSound is still being played, since that is up to the audio system, which goes by real time.
	if not self.currentlySpinningDown and not self:IsActivated() and not self:IsReloading() and self.wasActivated then
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
	self.wasActivated = self:IsActivated() and not self:IsReloading();

	if self.FiredFrame then
		self.RateOfFire = math.min(self.RateOfFire * self.increasePerShot, self.maxRateOfFire);
	elseif not self:IsActivated() then
		self.RateOfFire = math.max(self.RateOfFire * self.decreasePerFrame, self.minRateOfFire);
	end
end