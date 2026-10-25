function Create(self)
	self.startCaptureSound = CreateSoundContainer("Browncoat Capturable Start Capture", "Base.rte");
	self.capturingSound = CreateSoundContainer("Browncoat Capturable Capturing", "Base.rte");
	self.stopCaptureSound = CreateSoundContainer("Browncoat Capturable Stop Capture", "Base.rte");
	self.captureSuccessSound = CreateSoundContainer("Browncoat Capturable Capture Success", "Base.rte");
	
	self.capturingTimer = Timer();
	self.capturingDelay = 500;
	
	self.animationTimer = Timer();
	self.animationFrameTime = 500;
	
	self.startFrame = 0;
	self.endFrame = 1;
end

function ThreadedUpdate(self)
	if self.FXstartCapture then
		self.startCaptureSound:Play(self.Pos);
		self.capturingTimer:Reset();
	end
	
	if self.FXcapturing or self.Contested then
		self.startFrame = 3;
		self.endFrame = 9;
		self.animationFrameTime = 100;
		local sound = self.capturingSound;
		self.capturingDelay = 750;
		if self.dominantTeam == self.capturingTeam and not self.Contested then
			self.capturingDelay = 400;
		end
		if self.capturingTimer:IsPastSimMS(self.capturingDelay) then
			self.capturingTimer:Reset();
			sound:Play(self.Pos);
		end
	else
		-- AI team
		if self.Team == 1 then
			self.startFrame = 0;
			self.endFrame = 1;
			self.animationFrameTime = 500;
		else -- Human team
			self.startFrame = 11;
			self.endFrame = 28;
			self.animationFrameTime = 100;
		end
	end
	
	if self.animationTimer:IsPastSimMS(self.animationFrameTime) then
		self.Frame = math.max(self.Frame, self.startFrame);
		self.Frame = self.Frame + 1;
		if self.Frame > self.endFrame then
			self.Frame = self.startFrame;
		end
		self.animationTimer:Reset();
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