function Create(self)
	self.origActivationDelay = self.ActivationDelay;
	self.spinDownTimer = Timer();
	self.currentlySpinningDown = false;

	self.dingSound = false;
	self.ejectEffect = CreateAEmitter("Techion Giga Pulsar Magazine Eject Effect", "Techion.rte");
end

function Update(self)
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

	if self.Magazine then
		self.lastMag = self.Magazine;
		self.dingSound = false;
		if self.Magazine.RoundCount == 0 then
			self:Reload();
		end
	else
		if self.dingSound == false then
			if MovableMan:IsParticle(self.lastMag) then
				self.lastMag.Sharpness = 1;
				self.lastMag.Vel = self.lastMag.Vel + Vector(-10 * self.FlipFactor, 0):RadRotate(self.RotAngle);

				local effect = self.ejectEffect:Clone();
				effect.Pos = self.lastMag.Pos;
				effect.RotAngle = self.RotAngle;
				effect.HFlipped = self.HFlipped;
				MovableMan:AddParticle(effect);
			end
		end
		
		self.dingSound = true;
	end
end