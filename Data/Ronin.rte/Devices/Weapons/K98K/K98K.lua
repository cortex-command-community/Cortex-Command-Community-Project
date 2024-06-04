function OnFire(self)
	self.InheritedRotAngleTarget = self.recoilAngleSize * RangeRand(self.recoilAngleVariation, 1)
	self.rotationSpeed = 0.4;
	if self.RoundInMagCount == 0 then
		self.Reloadable = false;
	end
end

function OnReload(self)
	self.chamberSound:Stop(-1);
	if self.needsChamber then
		local shell = CreateMOSParticle("Casing Long");

		shell.Pos = self.Pos;
		shell.Vel = self.Vel + Vector(-6 * self.FlipFactor, -4):RadRotate(self.RotAngle);
		shell.Team = self.Team;
		MovableMan:AddParticle(shell);
	end
	self.needsChamber = false;
end

function Create(self)
	self.chamberSound = CreateSoundContainer("Chamber Ronin Kar98", "Browncoats.rte");
	self.boltBackSound = CreateSoundContainer("Bolt Back Ronin Kar98", "Browncoats.rte");
	self.boltForwardSound = CreateSoundContainer("Bolt Forward Ronin Kar98", "Browncoats.rte");
	self.preSound = CreateSoundContainer("Pre Ronin Kar98", "Browncoats.rte");
	self.roundInSound = CreateSoundContainer("Round In Ronin Kar98", "Browncoats.rte");

	self.reloadTimer = Timer();
	self.loadedShell = false;
	self.reloadCycle = false;

	self.reloadDelay = 300;
	self.origReloadTime = 900;
	
	-- for some reason if this is added to sim while facing leftwards, StanceOffset will actually be flipped.
	-- but not sharpstanceoffset...............

	
	self.origStanceOffset = Vector(self.StanceOffset.X*self.FlipFactor, self.StanceOffset.Y);
	self.origSharpStanceOffset = Vector(self.SharpStanceOffset.X, self.SharpStanceOffset.Y);
	
	self.origSupportOffset = Vector(self.SupportOffset.X, self.SupportOffset.Y);
	
	self.origShakeRange = self.ShakeRange;
	self.origSharpShakeRange = self.SharpShakeRange;
	
	self.origSharpLength = self.SharpLength;
	
	self.origReloadAngle = self.ReloadAngle

	self.ammoCounter = self.RoundInMagCount;
	self.maxAmmoCount = self.Magazine and self.Magazine.Capacity or 5; -- loading a game might mess this up, so... fall-back
	
	self.delayedFire = false
	self.delayedFireTimer = Timer();
	self.delayedFireTimeMS = 75;
	self.delayedFireEnabled = true;
	self.fireDelayTimer = Timer();
	self.activated = false;
	self.delayedFirstShot = true;
	
	self.InheritedRotAngleTarget = 0;
	self.InheritedRotAngleOffset = 0;
	self.recoilAngleSize = 0.5;
	self.recoilAngleVariation = 0.01;
	self.rotationSpeed = 0.1;
	self.rotFactor = math.pi;
	
	self.chamberTimer = Timer();
	self.chamberDelay = 300;
end

function Update(self)
	self.chamberSound.Pos = self.Pos;
	self.preSound.Pos = self.Pos;

	if self.FiredFrame then
		self.ammoCounter = self.ammoCounter - 1;
		self.ReloadStartSound = self.boltBackSound;
		self.chamberTimer:Reset();
		self.chamberDelay = 300;
		self.needsChamber = true;
		self.rotFactor = math.pi;
	end
	
	if IsAHuman(self:GetRootParent()) then
		self.parent = ToAHuman(self:GetRootParent())
	else
		self.parent = nil;
	end

	if self.Magazine then
		if not self.reloadCycle and self.needsChamber then
			self:Deactivate();
			self.delayedFire = false;
			
			if self.chamberTimer:IsPastSimMS(self.chamberDelay) then
			
				self.Reloadable = true;
				
				if self.ammoCounter == 0 then
					self:Reload();
				else	
					self.chamberSound:Play(self.Pos);
					self.rotateAnim = true;

					self.chamberTimer:Reset();
					self.rotateAnim = true;
					self.needsChamber = false;
					self.InheritedRotAngleTarget = 0;
					self.rotationSpeed = 0.1;
					
					local shell = CreateMOSParticle("Casing Long");

					shell.Pos = self.Pos;
					shell.Vel = self.Vel + Vector(-6 * self.FlipFactor, -4):RadRotate(self.RotAngle);
					shell.Team = self.Team;
					MovableMan:AddParticle(shell);
					
					self.chamberAnim = true;
				end
				
			end
		end
	
		if self.loadedShell then
			self.ReloadAngle = -0.15;
			self.BaseReloadTime = self.reloadDelay * 1.5;
			self.ammoCounter = math.min(self.Magazine.Capacity, self.ammoCounter + 1);
			self.Magazine.RoundCount = self.ammoCounter;
			self.loadedShell = false;
		end

		if self.reloadCycleEndNext == true then
			self.Magazine.RoundCount = self.ammoCounter;
			self.prematureCycleEnd = false;
			self.ReloadStartSound = self.boltBackSound;
			self.reloadCycleEndNext = false;
			self.BaseReloadTime = self.origReloadTime;
			self.ReloadAngle = self.origReloadAngle
			self.reloadCycle = false;
		end

		if self:IsActivated() then
			if self.reloadCycle then
				self.prematureCycleEnd = true;
			end
		end

		if self.reloadCycle and self.reloadTimer:IsPastSimMS(self.reloadDelay) then
			if self.parent then
				self:Reload();
			end
		end
		
		if self.rotateAnim then
			local minTime = 0;
			local maxTime = self.chamberDelay * 0.75;
			
			local factor = math.min(math.max(self.chamberTimer.ElapsedSimTimeMS - minTime, 0) / (maxTime - minTime), 1);
			
		end
	else
		self.chamberTimer:Reset();
		
		if self.reloadCycle ~= true then
			--self.chamberDelay = 300;
			if not self.Frame == 1 then
				self.openAnim = true;
			end
			self.InheritedRotAngleTarget = 0.1; -- not respected by the game currently
			self.ReloadEndSound = self.roundInSound;
			self.ReloadStartSound = nil;
			self.reloadCycle = true;
		end
		
		self.loadedShell = true;
		
		if self.ammoCounter == self.maxAmmoCount or self.prematureCycleEnd then
			self.closeAnim = true;
			self.ReloadAngle = 0.1;
			self.loadedShell = false;
			self.ReloadEndSound = self.boltForwardSound;
			self.reloadCycleEndNext = true;			
			self.BaseReloadTime = self.reloadDelay * 2;
		end		
	end
	
	if self.chamberAnim then
		local balance = 5 + math.abs(math.sin(self.parent.RotAngle) * 5);	--Laying down horizontally reduces swaying when pulling bolt
		self.Frame = 1;
		self.SupportOffset = Vector(-5, -1);
		local rotTotal = math.sin(self.rotFactor)/balance;

		local jointOffset = Vector(self.JointOffset.X * self.FlipFactor, self.JointOffset.Y):RadRotate(self.RotAngle);
		self.InheritedRotAngleTarget = rotTotal;

		self.rotFactor = self.rotFactor - (math.pi * 0.05);
		if self.rotFactor <= 0 then
			self.Frame = 0;
			self.chamberAnim = false;
			self.SupportOffset = self.origSupportOffset;
			self.rotFactor = 0;
		end
	end
	
	if self.openAnim then
		local minTime = 0;
		local maxTime = 300;
		
		local factor = math.min(math.max(self.reloadTimer.ElapsedSimTimeMS - minTime, 0) / (maxTime - minTime), 1);
		
		self.Frame = 0 + math.floor(factor * (1) + 0.5)
		self.SupportOffset = Vector(-5, -1);
		if self.Frame == 1 then
			self.openAnim = false;
			self.SupportOffset = self.origSupportOffset;
		end
	end
	
	if self.closeAnim then
		local minTime = 0;
		local maxTime = 600;
		
		local factor = math.min(math.max(self.chamberTimer.ElapsedSimTimeMS - minTime, 0) / (maxTime - minTime), 1);
		
		self.Frame = 1 - math.floor(factor * (1) + 0.5)
		self.SupportOffset = Vector(-5, -1);
		if self.Frame == 0 then
			self.closeAnim = false;
			self.SupportOffset = self.origSupportOffset;
		end
	end
	
	if self:DoneReloading() then
		self.reloadTimer:Reset();
		self.fireDelayTimer:Reset();
		self.activated = false;
		self.delayedFire = false;
	end
	
	if self.delayedFire and self.delayedFireTimer:IsPastSimMS(self.delayedFireTimeMS) then
		self:Activate();
		self.delayedFire = false
		self.delayedFirstShot = false;
	end

	local fire = self:IsActivated() and self.RoundInMagCount > 0 and not self.needsChamber;

	if self.delayedFirstShot == true then
		if self.RoundInMagCount > 0 then
			self:Deactivate()
		end
		
		if fire and not self:IsReloading() then
			if not self.Magazine or self.RoundInMagCount < 1 then
				--self:Activate()
			elseif not self.activated and not self.delayedFire and self.fireDelayTimer:IsPastSimMS(1 / (self.RateOfFire / 60) * 1000) then
				self.activated = true
				
				self.preSound:Play(self.Pos);
				
				self.fireDelayTimer:Reset()
				
				self.delayedFire = true
				self.delayedFireTimer:Reset()
			end
		else
			if self.activated then
				self.activated = false
			end
		end
	elseif fire == false then
		self.firstShot = true;
		self.delayedFirstShot = true;	
	end
	
	if self.InheritedRotAngleOffset ~= self.InheritedRotAngleTarget then
		self.InheritedRotAngleOffset = self.InheritedRotAngleOffset - (self.rotationSpeed * (self.InheritedRotAngleOffset - self.InheritedRotAngleTarget))
	end
	
	if self.InheritedRotAngleTarget > 0 then
		self.InheritedRotAngleTarget = math.max(self.InheritedRotAngleTarget - TimerMan.DeltaTimeSecs, 0);
	end
end