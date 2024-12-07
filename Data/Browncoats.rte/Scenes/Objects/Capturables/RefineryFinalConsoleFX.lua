function Create(self)
	self.startCaptureSound = CreateSoundContainer("Browncoat Final Console Start Capture", "Base.rte");
	self.capturingSound = CreateSoundContainer("Browncoat Final Console Capturing", "Base.rte");
	self.stopCaptureSound = CreateSoundContainer("Browncoat Final Console Stop Capture", "Base.rte");
	self.captureSuccessSound = CreateSoundContainer("Browncoat Final Console Final Sequence", "Base.rte");
	
	self.capturingTimer = Timer();
	self.capturingDelay = 500;
end

function Update(self)
	if self.FXstartCapture then
		self.startCaptureSound:Play(self.Pos);
		self.capturingTimer:Reset();
	end
	if self.FXcapturing or self.Contested then
		local sound = self.capturingSound;
		self.capturingDelay = 750;
		if self.dominantTeam == self.capturingTeam and not self.Contested then
			self.capturingDelay = 400;
		end
		if self.capturingTimer:IsPastSimMS(self.capturingDelay) then
			self.capturingTimer:Reset();
			sound:Play(self.Pos);
		end
	end
	if self.FXstopCapture then
		self.stopCaptureSound:Play(self.Pos);
	end
	if self.FXcaptureSuccess then
		self.captureSuccessSound:Play(self.Pos);
	end
	if self.FXcaptureUncapped then
		self.capturingTimer:Reset();
	end

	self.FXstartCapture = false;
	self.FXstopCapture = false;
	self.FXcaptureSuccess = false;
	self.FXcaptureUncapped = false;			
end