function OnFire(self)
	if not self.Firing then
		self.Firing = true;
		self.startShotSound:Play(self.Pos);
	end
end

function Create(self)
	self.spinUpSound = CreateSoundContainer("Spin Up Browncoat RG-1100", "Browncoat.rte");
	self.spinDownSound = CreateSoundContainer("Spin Down Browncoat RG-1100", "Browncoat.rte");
	self.startShotSound = CreateSoundContainer("Start Shot Browncoat RG-1100", "Browncoat.rte");
	self.tailSound = CreateSoundContainer("Tail Browncoat RG-1100", "Browncoat.rte");
	
	self.spinUpTimer = Timer();
	self.spinUpTime = 1000;
end

function Update(self)
	local fire = self:IsActivated() and self.RoundInMagCount > 0;
	
	if fire then
		if not self.spinUpTimer:IsPastSimMS(self.spinUpTime) then
			if not self.spinningUp then
				self.spinningUp = true;
				self.spinUpSound:Play(self.Pos);
			end
			self:Deactivate();
		end
			
	else
		if self.Firing then
			self.spinningUp = false;
			self.Firing = false;
			self.tailSound:Play(self.Pos);
			self.spinDownSound:Play(self.Pos);
		end
		if self.spinningUp then
			self.spinningUp = false;
			self.spinDownSound:Play(self.Pos);
			self.spinUpSound:FadeOut(100);
		end
		self.spinUpTimer:Reset();
	end
end

function Destroy(self)
	self.spinUpSound:Stop(-1);
	self.spinDownSound:Stop(-1);
end