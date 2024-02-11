function Create(self)
	--Suffer from fire rate loss when firing rapidly
	self.fireRatePenaltyPerShot = 75;
	self.fireRateRevertIncrement = 1;
	self.fireRateRevertTimer = Timer();
end

function ThreadedUpdate(self)
	if not self.origRateOfFire then	--Check original stats on Update() to include global script changes
		self.origRateOfFire = self.RateOfFire;
	end

	if self.FiredFrame then
		local parent = self:GetParent() or self;

		self.InheritedRotAngleOffset = self.InheritedRotAngleOffset + RangeRand(1.2, 1.5)/math.sqrt(1 + parent.Mass + parent.Material.StructuralIntegrity * 0.1);
		self.RateOfFire = math.max(self.RateOfFire - self.fireRatePenaltyPerShot, 1);

		self.fireRateRevertTimer:Reset();
	elseif self.RateOfFire < self.origRateOfFire then
		self.RateOfFire = math.min(self.RateOfFire * (1 + self.fireRateRevertIncrement * 0.01) + self.fireRateRevertIncrement, self.origRateOfFire);
		self.fireRateRevertTimer:Reset();
	end

	if self.InheritedRotAngleOffset > 0 then
		self.InheritedRotAngleOffset = self.InheritedRotAngleOffset - 0.0001 * (self.RateOfFire/(1 + self.InheritedRotAngleOffset));

		if self.InheritedRotAngleOffset < 0 then
			self.InheritedRotAngleOffset = 0;
		end
	end
end