function Create(self)
	self.reloadTimer = Timer();
	self.loadedShell = false;
	self.reloadCycle = false;

	self.reloadDelay = 75;

	self.ammoCounter = self.RoundInMagCount;
end

function ThreadedUpdate(self)
	if self.FiredFrame then
		self.ammoCounter = self.ammoCounter - 1;
	end
	if self.Magazine then
		if self.loadedShell then
			self.ammoCounter = self.ammoCounter + 1;
			self.Magazine.RoundCount = self.ammoCounter;
			self.loadedShell = false;
		end
		if self:IsFull() then
			self.reloadCycle = false;
		end
		if self.reloadCycle and self.reloadTimer:IsPastSimMS(self.reloadDelay) then
			local actor = self:GetRootParent();
			if MovableMan:IsActor(actor) then
				self:Reload();
			end
		end
	else
		self.reloadTimer:Reset();
		self.reloadCycle = true;
		self.loadedShell = true;
	end
	if self:IsActivated() then
		self.reloadCycle = false;
	end
end