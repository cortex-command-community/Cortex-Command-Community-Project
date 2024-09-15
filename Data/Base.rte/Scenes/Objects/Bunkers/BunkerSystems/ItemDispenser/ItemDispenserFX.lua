function Create(self)
	self.dispenseSound = CreateSoundContainer("Item Dispenser Dispense", "Base.rte");
	self.reloadSound = CreateSoundContainer("Item Dispenser Reload", "Base.rte");
end

function ThreadedUpdate(self)
	if self.FXstartCapture then
		self.startCaptureSound:Play(self.Pos);
		self.capturingTimer:Reset();
	end

	if self.FXcapturing or self.Contested then
		local sound = self.capturingSound;
		self.capturingDelay = 500;
		if self.dominantTeam == self.capturingTeam and not self.Contested then
			self.capturingDelay = 250;
		end
		if self.capturingTimer:IsPastSimMS(self.capturingDelay) then
			self.capturingTimer:Reset();
			sound:Play(self.Pos);
		end
	end

	if self.FXDispensed then
		self.dispenseSound:Play(self.Pos);
	end

	if self.FXReloaded then
		self.reloadSound:Play(self.Pos);
	end

	self.FXDispensed = false;
	self.FXReloaded = false;
end