function Create(self)
	self.pullTimer = Timer();
	self.loaded = false;
	self.rotFactor = math.pi;

	self.cockSound = CreateSoundContainer("Ronin SPAS 12 Cock Sound", "Ronin.rte");
end

function Update(self)
	local actor = self:GetRootParent();
	if not (actor and IsAHuman(actor)) then
		self.pullTimer:Reset();
	end
	
	if self.FiredFrame and self.RoundInMagCount == 0 then
		self.loaded = false;
		self.playedSound = false;
		self.rotFactor = math.pi;
	end

	if not self.loaded and self.RoundInMagCount > 0 and not self.reloadCycle then
		self:Deactivate();
		if self.pullTimer:IsPastSimMS(30000/self.RateOfFire) then
			if not self.playedSound then
				self.cockSound:Play(self.Pos);
				self.playedSound = true;
			end

			self.Frame = 1;
			self.SupportOffset = Vector(-2, 4);
			local rotTotal = math.sin(self.rotFactor) * 0.2;
			self.RotAngle = self.RotAngle + self.FlipFactor * rotTotal;
			local jointOffset = Vector(self.JointOffset.X * self.FlipFactor, self.JointOffset.Y):RadRotate(self.RotAngle);
			self.Pos = self.Pos - jointOffset + Vector(jointOffset.X, jointOffset.Y):RadRotate(-rotTotal * self.FlipFactor);
			self.rotFactor = self.rotFactor - 0.15;
		end

		if self.rotFactor <= 0 then
			self.loaded = true;
			self.Frame = 0;
			self.SupportOffset = Vector(1, 3);
			self.rotFactor = 0;
		end
	else
		self.pullTimer:Reset();
	end
end