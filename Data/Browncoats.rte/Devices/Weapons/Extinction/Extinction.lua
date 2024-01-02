function OnFire(self)

	self.InheritedRotAngleTarget = self.recoilAngleSize * RangeRand(self.recoilAngleVariation, 1)

end

function Create(self)

	self.fanFireSound = CreateSoundContainer("Fanfire Browncoat R-500", "Browncoats.rte");
	self.fanFireSound.Volume = 0.6
	self.cockSound = CreateSoundContainer("Cock Browncoat R-500", "Browncoats.rte");
	self.reloadStartNoEjectSound = CreateSoundContainer("Reload Start No Eject Browncoat R-500", "Browncoats.rte");
	self.reloadStartSound = CreateSoundContainer("Reload Start Browncoat R-500", "Browncoats.rte");
	self.reloadEndSound = CreateSoundContainer("Reload End Browncoat R-500", "Browncoats.rte");
	self.preSound = CreateSoundContainer("Pre Browncoat R-500", "Browncoats.rte");
	self.roundInSound = CreateSoundContainer("Round In Browncoat R-500", "Browncoats.rte");

	self.reloadTimer = Timer();
	self.loadedShell = false;
	self.reloadCycle = false;

	self.reloadDelay = 150;
	self.origReloadTime = 1200;
	
	self.origStanceOffset = self.StanceOffset;
	self.origSharpStanceOffset = self.SharpStanceOffset;
	
	self.origShakeRange = self.ShakeRange;
	self.origSharpShakeRange = self.SharpShakeRange;
	
	self.origSharpLength = self.SharpLength;
	
	self.origReloadAngle = self.ReloadAngle
	self.origOneHandedReloadAngle = self.OneHandedReloadAngle;

	self.ammoCounter = self.RoundInMagCount;
	self.maxAmmoCount = self.Magazine and self.Magazine.Capacity or 5; -- loading a game might mess this up, so... fall-back
	
	self.shellsToEject = 0;
	self.shellMOSParticle = self:StringValueExists("CylinderShellMOSParticle") and self:GetStringValue("CylinderShellMOSParticle") or "Base.rte/Casing";
	self.shellMOSRotating = self:StringValueExists("CylinderShellMOSRotating") and self:GetStringValue("CylinderShellMOSRotating") or nil;
	
	self.delayedFire = false
	self.delayedFireTimer = Timer();
	self.delayedFireTimeMS = 75;
	self.delayedFireEnabled = true;
	self.fireDelayTimer = Timer();
	self.activated = false;
	self.delayedFirstShot = true;
	
	self.fanFireTimer = Timer();
	self.fanFireHoldTime = 200;
	self.fanFire = false;
	
	self.fanFireStanceOffset = Vector(7, 7);
	self.fanFireSharpStanceOffset = self.fanFireStanceOffset;
	self.fanFireShakeRange = 6;
	self.fanFireSharpShakeRange = 6;
	self.fanFireSharpLength = 100;
	
	self.InheritedRotAngleTarget = 0;
	self.InheritedRotAngleOffset = 0;
	self.recoilAngleSize = 0.45;
	self.recoilAngleVariation = 0.02;
	self.rotationSpeed = 0.1;
	
	self.cockTimer = Timer();
	self.cockDelay = 300;

end

function Update(self)

	self.fanFireSound.Pos = self.Pos;
	self.cockSound.Pos = self.Pos;
	self.preSound.Pos = self.Pos;
	
	self.Reloadable = true;
	
	if IsAHuman(self:GetRootParent()) then
		self.parent = ToAHuman(self:GetRootParent())
		if self.parent.FGArm and self:GetParent().UniqueID == self.parent.FGArm.UniqueID then
			if self.parent.BGArm and self.parent.EquippedBGItem then
				-- dumb edge case fix...
				self.reloadDelay = 135;
				self.otherHandGun = self.parent.EquippedBGItem;
			else
				self.otherHandGun = nil;
			end
		else
			if self.parent.FGArm and self.parent.EquippedItem then
				self.otherHandGun = self.parent.EquippedItem;
				self.reloadDelay = 175;
			else
				self.otherHandGun = nil;
			end
		end
		
		if self.otherHandGun then
			if self.otherHandGun:IsReloading() then
				self.Reloadable = false;
			end
		end
		
		if self.otherHandGun or not self.parent.BGArm or not self.parent.FGArm then
			self.FullAuto = false;
			if self.fanFire then
				--self.FullAuto = false;
				self.fanFire = false;
				self.cockDelay = 300;
				self.StanceOffset = self.origStanceOffset;
				self.SharpStanceOffset = self.origStanceOffset;
				--self.SharpLength = self.origSharpLength;
				self.ShakeRange = self.origShakeRange;
				self.SharpShakeRange = self.origShakeRange;
			end
		else
			self.FullAuto = true;
			self.reloadDelay = 150;
		end
	else
		self.parent = nil;
	end

	if self.FiredFrame then
		self.fanFireSound:Stop(-1);
		self.ammoCounter = self.ammoCounter - 1;
		self.shellsToEject = self.shellsToEject + 1;
		self.ReloadStartSound = self.reloadStartSound;
		self.Frame = 1;
		self.cockTimer:Reset();
		self.cockDelay = 300;
		if self.fanFire then
			self.cockDelay = 100;
		end
	end
	if self.Magazine then
	
		if self.FullAuto and not self.reloadCycle and self:IsActivated() and self.fanFireTimer:IsPastSimMS(self.fanFireHoldTime) then
			self.fanFire = true;
			--self.FullAuto = true;
			self.delayedFirstShot = true;
			self.cockDelay = 100;
			self.StanceOffset = self.fanFireStanceOffset;
			self.SharpStanceOffset = self.fanFireStanceOffset;
			--self.SharpLength = self.fanFireSharpLength;
			self.ShakeRange = self.fanFireShakeRange;
			self.SharpShakeRange = self.fanFireShakeRange;
		elseif not self:IsActivated() then
			self.fanFireTimer:Reset();
			if self.fanFire then
				--self.FullAuto = false;
				self.fanFire = false;
				self.cockDelay = 300;
				self.StanceOffset = self.origStanceOffset;
				self.SharpStanceOffset = self.origStanceOffset;
				--self.SharpLength = self.origSharpLength;
				self.ShakeRange = self.origShakeRange;
				self.SharpShakeRange = self.origShakeRange;
			end
		end
	
		if not self.reloadCycle and self.Frame == 1 then
			self:Deactivate();
			self.delayedFire = false;
			
			if self.cockTimer:IsPastSimMS(self.cockDelay) then
				if self.fanFire then
					--self.fanFireSound:Play(self.Pos);
				else
					self.cockSound:Play(self.Pos);
				end
				self.InheritedRotAngleTarget = 0;
				self.Frame = 0;
			end
		end
	
		if self.loadedShell then
			self.ReloadAngle = -0.25;
			self.OneHandedReloadAngle = -0.4;
			self.BaseReloadTime = self.reloadDelay * 3;
			self.ammoCounter = math.min(self.Magazine.Capacity, self.ammoCounter + 1);
			self.Magazine.RoundCount = self.ammoCounter;
			self.loadedShell = false;
		end
		if self.reloadCycleEndNext == true then
			self.Magazine.RoundCount = self.ammoCounter;
			self.prematureCycleEnd = false;
			self.ReloadStartSound = self.reloadStartNoEjectSound;
			self.reloadCycleEndNext = false;
			self.BaseReloadTime = self.origReloadTime;
			self.ReloadAngle = self.origReloadAngle
			self.OneHandedReloadAngle = self.origOneHandedReloadAngle;
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
	else
	
		self.cockTimer:Reset();
	
		self.reloadTimer:Reset();
		
		if self.reloadCycle ~= true then
			--self.cockDelay = 300;
			self.InheritedRotAngleTarget = 0.1; -- not respected by the game currently
			self.ReloadEndSound = self.roundInSound;
			self.ReloadStartSound = nil;
			self.reloadCycle = true;
			if self.shellsToEject > 0 then
				for i = 1, self.shellsToEject do
					local shell = self.shellMOSRotating and CreateMOSRotating(self.shellMOSRotating) or CreateMOSParticle(self.shellMOSParticle);
					shell.Pos = self.Pos;
					shell.Vel = self.Vel + Vector(RangeRand(-3, 0) * self.FlipFactor, 0):RadRotate(self.RotAngle + RangeRand(-0.3, 0.3));
					shell.AngularVel = RangeRand(-1, 1);
					MovableMan:AddParticle(shell);
				end
			end
		end
		
		self.shellsToEject = 0;
		self.loadedShell = true;
		
		if self.ammoCounter == self.maxAmmoCount or self.prematureCycleEnd then
			self.ReloadAngle = 0.1;
			self.OneHandedReloadAngle = 0.2;
			self.loadedShell = false;
			self.ReloadEndSound = self.reloadEndSound;
			self.reloadCycleEndNext = true;			
			self.BaseReloadTime = self.reloadDelay;
		end		
		
	end
	
	if self:DoneReloading() then
		self.fireDelayTimer:Reset();
		self.activated = false;
		self.delayedFire = false;
		
	end
	
	if self.delayedFire and self.delayedFireTimer:IsPastSimMS(self.delayedFireTimeMS) then
		self:Activate();
		self.delayedFire = false
		self.delayedFirstShot = false;
	end

	local fire = self:IsActivated() and self.RoundInMagCount > 0 and self.Frame == 0;

	if self.delayedFirstShot == true then
		if self.RoundInMagCount > 0 then
			self:Deactivate()
		end
		
		if fire and not self:IsReloading() then
			if not self.Magazine or self.RoundInMagCount < 1 then
				--self:Activate()
			elseif not self.activated and not self.delayedFire and self.fireDelayTimer:IsPastSimMS(1 / (self.RateOfFire / 60) * 1000) then
				self.activated = true
				
				if self.fanFire then
					self.fanFireSound:Play(self.Pos);
				else
					self.preSound:Play(self.Pos);
				end
				
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