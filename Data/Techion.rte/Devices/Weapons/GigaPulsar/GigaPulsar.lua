function Create(self)
	self.origActivationDelay = self.ActivationDelay;
	self.spinDownTimer = Timer();
	self.currentlySpinningDown = false;
	self.wasActivated = false;

	self.dingSound = false;
	self.ejectEffect = CreateAEmitter("Techion Giga Pulsar Magazine Eject Effect", "Techion.rte");
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

				self.effect = self.ejectEffect:Clone();
				self.effect.Pos = self.lastMag.Pos;
				self.effect.RotAngle = self.RotAngle;
				self.effect.HFlipped = self.HFlipped;
				self:RequestSyncedUpdate();
			end
		end
		
		self.dingSound = true;
	end
end

function SyncedUpdate(self)
	if self.effect then
		MovableMan:AddParticle(self.effect);
		self.effect = nil;
	end
end