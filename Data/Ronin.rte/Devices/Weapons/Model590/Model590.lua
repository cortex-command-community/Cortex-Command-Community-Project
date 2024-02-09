function Create(self)
	self.pullTimer = Timer();
	self.loaded = false;
	self.rotFactor = math.pi;

	self.cockSound = CreateSoundContainer("Ronin Model 590 Cock Sound", "Ronin.rte");
end

function Update(self)
	local actor = self:GetRootParent();
	if not (actor and IsAHuman(actor)) then
		self.pullTimer:Reset();
	end

	if self.FiredFrame then
		self.shell = CreateMOSParticle("Shell");
		self.loaded = false;
		self.playedSound = false;
		self.rotFactor = math.pi;
	end

	if not self.loaded and self.RoundInMagCount > 0 and not self.reloadCycle then
		if self.pullTimer:IsPastSimMS(15000/self.RateOfFire) then
			if not self.playedSound then
				--self.cockSound:Play(self.Pos);	--TODO: Separate the cocking sound from FireSound
				self.playedSound = true;
			end
			if self.shell then
				self.shell.Pos = self.Pos;
				self.shell.Vel = self.Vel + Vector(-6 * self.FlipFactor, -4):RadRotate(self.RotAngle);
				self.shell.Team = self.Team;
				MovableMan:AddParticle(self.shell);
				self.shell = nil;
			end
			self.Frame = 1;
			self.SupportOffset = Vector(-2, 4);
			local rotTotal = math.sin(self.rotFactor) * 0.2;
			self.RotAngle = self.RotAngle + self.FlipFactor * rotTotal;
			local jointOffset = Vector(self.JointOffset.X * self.FlipFactor, self.JointOffset.Y):RadRotate(self.RotAngle);
			self.Pos = self.Pos - jointOffset + Vector(jointOffset.X, jointOffset.Y):RadRotate(-rotTotal * self.FlipFactor);
			self.rotFactor = self.rotFactor - math.pi * 0.0005 * self.RateOfFire;
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