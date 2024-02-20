-- This script incorporates Filipawn Industries code
-- There are likely better ways of doing a lot of this, potentially even standardizing it so it can be easily used more widely

-- Last worked on 25/09/2023

function OnAttach(self)
	if self.bossMode then
		--self.Heat = 0;
		self.bossChamberAnim = true;
		self.bossChamberTimer:Reset();
		self.bossChamberSoundPlayed = false;
	end
end

function OnFire(self)
	CameraMan:AddScreenShake(self.screenShakeAmount, self.Pos);
	
	self.mechTailSound:Play(self.Pos);
	
	if self.firstShot then
		self.firstShotSound:Play(self.Pos);
		self.firstShot = false;
		self.overheatLoopSound:Play(self.Pos);
		
		if self.bossMode then
			-- extra fx
			CameraMan:AddScreenShake(3, self.Pos);
			
			for i = 1, 1 do
				local particle = CreateMOPixel("Glow Explosion Huge", "Base.rte");
				particle.HitsMOs = false;
				particle.Lifetime = math.random(10, 50);
				particle.Vel = self.Vel
				particle.Pos = self.Pos;
				MovableMan:AddParticle(particle);
			end					
			
			for i = 1, 1 do
				local particle = CreateMOSParticle("Fire Puff Medium", "Base.rte");
				particle.HitsMOs = false;
				particle.Lifetime = math.random(250, 600);
				particle.Vel = self.Vel + Vector(math.random(-200, 200)/100, -3);
				particle.Pos = self.MuzzlePos;
				MovableMan:AddParticle(particle);
			end
		end		
	else
		self.shotSound:Play(self.Pos);
	end
	
	self.InheritedRotAngleTarget = self.InheritedRotAngleOffset + (self.recoilAngleSize * RangeRand(self.recoilAngleVariation, 1))
	
	-- Heat mechanics
	
	-- unsteady rate of fire
	--self.RateOfFire = self.originalRateOfFire + math.max(0, math.random(-100, 0)* (self.Heat-70)/100)

	-- FireSound is our mechanical sound, have it rise as we overheat
	self.FireSound.Volume = math.min(1.5, 0.8 + self.Heat / 200);
	
	if self.Heat > self.heatLimit * self.heatWarningLimitMultiplier then
		self.preSound.Volume = 1.5;
		if IsActor(self:GetRootParent()) and ToActor(self:GetRootParent()):IsPlayerControlled() then
			self.preSound.PanningStrengthMultiplier = 0;
		else
			self.preSound.PanningStrengthMultiplier = 1;
		end
		self.preSound:Play(self.Pos);
		local particle = CreateMOSParticle("Side Thruster Blast Ball 1", "Base.rte");
		particle.HitsMOs = false;
		particle.Lifetime = math.random(250, 600);
		particle.Vel = self.Vel + Vector(math.random(-100, 100)/100, -4);
		particle.Pos = self.Pos;
		MovableMan:AddParticle(particle);
	end
	
	self.Heat = math.min(self.heatLimit, self.Heat + self.heatPerShot);
	
	if self.Heat == self.heatLimit then
		self.overheatFixSound:Play(self.Pos);
		self.Overheated = true;
		self.overheatCount = self.overheatCount + 1;
		self:SetNumberValue("Busy", 1);
		self.overheatStuckShell = true;
		
		self.overheatFixTimer:Reset();
		
		for i = 1, 10 do
			local particle = CreateMOSParticle("Tiny Smoke Ball 1", "Base.rte");
			particle.Lifetime = math.random(250, 600);
			particle.Vel = self.Vel + Vector(math.random(-100, 100)/100, -2);
			particle.Pos = self.Pos;
			MovableMan:AddParticle(particle);
		end		
		
		for i = 1, 5 do
			local particle = CreateMOSParticle("Small Smoke Ball 1", "Base.rte");
			particle.Lifetime = math.random(250, 600);
			particle.Vel = self.Vel + Vector(math.random(-100, 100)/100, -2);
			particle.Pos = self.Pos;
			MovableMan:AddParticle(particle);
		end
		
		for i = 1, 3 do
			local particle = CreateMOSParticle("Side Thruster Blast Ball 1", "Base.rte");
			particle.HitsMOs = false;
			particle.Lifetime = math.random(250, 600);
			particle.Vel = self.Vel + Vector(math.random(-200, 200)/100, -3);
			particle.Pos = self.Pos;
			MovableMan:AddParticle(particle);
		end
		
		if self.bossMode then
			-- extra fx
			CameraMan:AddScreenShake(15, self.Pos);
			
			for i = 1, 1 do
				local particle = CreateMOPixel("Glow Explosion Huge", "Base.rte");
				particle.HitsMOs = false;
				particle.Lifetime = math.random(10, 50);
				particle.Vel = self.Vel
				particle.Pos = self.Pos;
				MovableMan:AddParticle(particle);
			end			
			
			for i = 1, 1 do
				local particle = CreateMOSParticle("Fire Puff Large", "Base.rte");
				particle.HitsMOs = false;
				particle.Lifetime = math.random(250, 600);
				particle.Vel = self.Vel + Vector(math.random(-200, 200)/100, -3);
				particle.Pos = self.Pos;
				MovableMan:AddParticle(particle);
			end			
			
			for i = 1, 3 do
				local particle = CreateMOSParticle("Fire Puff Medium", "Base.rte");
				particle.HitsMOs = false;
				particle.Lifetime = math.random(250, 600);
				particle.Vel = self.Vel + Vector(math.random(-200, 200)/100, -3);
				particle.Pos = self.Pos;
				MovableMan:AddParticle(particle);
			end
		end
	end
end

function Create(self)
	self.preSound = CreateSoundContainer("Pre Browncoat MG-85", "Browncoat.rte");
	
	self.firstShotSound = CreateSoundContainer("First Shot Browncoat MG-85", "Browncoat.rte");
	self.shotSound = CreateSoundContainer("Shot Browncoat MG-85", "Browncoat.rte");
	
	self.overheatLoopSound = CreateSoundContainer("Overheat Loop Browncoat MG-85", "Browncoat.rte");
	self.overheatFixSound = CreateSoundContainer("Overheat Fix Browncoat MG-85", "Browncoat.rte");
	
	self.mechTailSound = CreateSoundContainer("Mech Tail Browncoat MG-85", "Browncoat.rte");
	
	self.firstShot = true;
	
	self.delayedFire = false
	self.delayedFireTimer = Timer();
	self.delayedFireTimeMS = 80;
	self.delayedFireEnabled = true;
	self.fireDelayTimer = Timer();
	self.activated = false;
	self.delayedFirstShot = true;
	
	self.originalStanceOffset = Vector(self.StanceOffset.X, self.StanceOffset.Y);
	self.originalSharpStanceOffset = Vector(self.SharpStanceOffset.X, self.SharpStanceOffset.Y);
	self.originalSupportOffset = Vector(self.SupportOffset.X, self.SupportOffset.Y);
	self.originalRateOfFire = self.RateOfFire
	
	self.heatFXTimer = Timer();
	self.Heat = 0;
	self.heatLimit = 100;
	self.heatWarningLimitMultiplier = 0.7;
	self.heatPerShot = 2.5;
	
	self.overheatCount = 0;
	self.overheatFixTimer = Timer();
	self.overheatFixDelay = 2300;
	
	self.InheritedRotAngleTarget = 0;
	self.InheritedRotAngleOffset = 0;
	self.recoilAngleSize = 5;
	self.recoilAngleVariation = 0.2;
	self.rotationSpeed = 0.01;
	
	self.screenShakeAmount = 3;
	
	if self:NumberValueExists("Boss Mode") then
		self.bossChamberSound = CreateSoundContainer("Boss Chamber Browncoat MG-85", "Browncoat.rte");
		
		self.firstShotSound.Volume = 1.3;
	
		self.bossMode = true;
		self.Magazine.RoundCount = 300;
		self.heatLimit = 200;
		self.heatWarningLimitMultiplier = 0.6;
		self.heatPerShot = 5;
		
		self.screenShakeAmount = 6;
		
		self.Reloadable = false;
		
		self.bossChamberAnim = false;
		self.bossChamberTimer = Timer();
		self.bossChamberDelay = 3000;
		
		self.bossChamberSoundPlayed = false;
	end
end

function Update(self)
	self.preSound.Pos = self.Pos;
	self.overheatLoopSound.Pos = self.Pos;
	self.overheatFixSound.Pos = self.Pos;
	self.mechTailSound.Pos = self.Pos;
	
	self.overheatLoopSoundVolumeTarget = math.min(1.5, self.Heat * 3 / 225) -- will rise to 1.3 with player, a little more with boss
	
	if self.overheatLoopSound.Volume < self.overheatLoopSoundVolumeTarget then
		self.overheatLoopSound.Volume = math.min(self.overheatLoopSoundVolumeTarget, self.overheatLoopSound.Volume + TimerMan.DeltaTimeSecs * 10);
	elseif self.overheatLoopSound.Volume > self.overheatLoopSoundVolumeTarget then
		self.overheatLoopSound.Volume = math.max(self.overheatLoopSoundVolumeTarget, self.overheatLoopSound.Volume - TimerMan.DeltaTimeSecs * 0.33);
	end
	
	if self.bossMode and self.RoundInMagCount == 0 then
		self.Reloadable = true;
		self:Reload();
	end
	
	if self:IsReloading() then
		self.fireDelayTimer:Reset();
		self.activated = false;
		self.delayedFire = false;
	end
	
	if self:DoneReloading() then
		self.overheatCount = 0;
		self.fireDelayTimer:Reset();
		self.activated = false;
		self.delayedFire = false;
		
		if self.bossMode == true then
			self.Reloadable = false;
			self.Magazine.RoundCount = 300;
			self.bossChamberAnim = true;
			self.bossChamberTimer:Reset();
			self.bossChamberSoundPlayed = false;
			self:SetNumberValue("Busy", 1);
		end
		
	end
	
	if self.delayedFire and self.delayedFireTimer:IsPastSimMS(self.delayedFireTimeMS) then
		self:Activate();
		self.delayedFire = false
		self.delayedFirstShot = false;
	end

	local fire = self:IsActivated() and self.RoundInMagCount > 0 and not self.Overheated and not self.bossChamberAnim;

	if self.delayedFirstShot == true then
		if self.RoundInMagCount > 0 then
			self:Deactivate()
		end
		
		--if self.parent:GetController():IsState(Controller.WEAPON_FIRE) and not self:IsReloading() then
		if fire and not self:IsReloading() then
			if not self.Magazine or self.RoundInMagCount < 1 then
				--self:Reload()
				self:Activate()
			elseif not self.activated and not self.delayedFire and self.fireDelayTimer:IsPastSimMS(1 / (self.RateOfFire / 60) * 1000) then
				self.activated = true
				
				self.preSound.Volume = 1;
				self.preSound.PanningStrengthMultiplier = 1;
				self.preSound:Play(self.Pos);
				
				self.fireDelayTimer:Reset()
				
				self.delayedFire = true
				self.delayedFireTimer:Reset()
			end
		else
			self.Heat = math.max(0, self.Heat - TimerMan.DeltaTimeSecs * 60);
			
			if self.activated then
				self.activated = false
			end
		end
	elseif fire == false then
		self.firstShot = true;
		self.delayedFirstShot = true;
	end
	
	if self.Overheated then
		self.Heat = math.max(0, self.Heat - TimerMan.DeltaTimeSecs * 30);
		
		if self.overheatFixTimer:IsPastSimMS(self.overheatFixDelay) then
			self.Heat = 0;
		
			self.rotationSpeed = 0.01;
		
			self.Overheated = false;
			self:RemoveNumberValue("Busy");
			
			if self.bossMode and self.overheatCount == 3 then
				self.Reloadable = true;
				self.overheatCount = 0;
				self:Reload();
			end
			
			self.SupportOffset = self.originalSupportOffset;
			
			self.InheritedRotAngleTarget = 0;
		elseif self.overheatFixTimer:IsPastSimMS(self.overheatFixDelay / 1.5) then
			self.rotationSpeed = 0.03;
			
			self.SupportOffset = Vector(-1, -2);
			
			self.InheritedRotAngleTarget = 0;
		elseif self.overheatFixTimer:IsPastSimMS(self.overheatFixDelay / 2) then
			if self.overheatStuckShell == true then
				self.SupportOffset = Vector(-5, -2);
				
				local shell = CreateAEmitter("Smoking Large Casing", "Base.rte");
				shell.Pos = self.Pos+Vector(0,-1):RadRotate(self.RotAngle);
				shell.Vel = self.Vel+Vector(-math.random(2,4)*self.FlipFactor,-math.random(5,6)):RadRotate(self.RotAngle);
				shell.AngularVel = 10 * self.FlipFactor;
				shell.RotAngle = self.RotAngle
				shell.HFlipped = self.HFlipped
				MovableMan:AddParticle(shell);
				
				self.overheatStuckShell = false;
				
				self.InheritedRotAngleTarget = -0.05;
			end
		elseif self.overheatFixTimer:IsPastSimMS(self.overheatFixDelay / 4) then
			self.rotationSpeed = 0.05;
		
			self.SupportOffset = Vector(0, -2)
			
			self.InheritedRotAngleTarget = -0.15;
		end
	end			
	
	if self.bossChamberAnim then
		if self.bossChamberTimer:IsPastSimMS(self.bossChamberDelay) then
		
			self.bossChamberAnim = false;
			
			self:RemoveNumberValue("Busy");
		
			self.rotationSpeed = 0.01;
			
			self.SupportOffset = self.originalSupportOffset;
			
			self.InheritedRotAngleTarget = 0;
		elseif self.bossChamberTimer:IsPastSimMS(self.bossChamberDelay / 1.5) then
			self.rotationSpeed = 0.02;
			
			self.SupportOffset = Vector(-1, -2);
			
			self.InheritedRotAngleTarget = 0.25;
		elseif self.bossChamberTimer:IsPastSimMS(self.bossChamberDelay / 2) then
			self.SupportOffset = Vector(-5, -2);
			
			self.InheritedRotAngleTarget = 3;
			
			if self.bossChamberSoundPlayed == false then
				self.bossChamberSoundPlayed = true;
				self.bossChamberSound:Play(self.Pos);
			end	
		elseif self.bossChamberTimer:IsPastSimMS(self.bossChamberDelay / 4) then
		
			self.rotationSpeed = 0.01;
		
			self.SupportOffset = Vector(0, -2)
			
			self.InheritedRotAngleTarget = 3;
		end
	end	
	
	if self.heatFXTimer:IsPastSimMS(500) then
		self.heatFXTimer:Reset();
		
		for i = 1, self.Heat / 20 do
			local particle = CreateMOSParticle("Tiny Smoke Ball 1", "Base.rte");
			particle.Lifetime = math.random(250, 600);
			particle.Vel = self.Vel + Vector(math.random(-100, 100)/100, -1);
			particle.Pos = self.Pos;
			MovableMan:AddParticle(particle);
		end
		
		for i = 1, self.Heat / 50 do
			local particle = CreateMOSParticle("Small Smoke Ball 1", "Base.rte");
			particle.Lifetime = math.random(250, 600);
			particle.Vel = self.Vel + Vector(math.random(-100, 100)/100, -1);
			particle.Pos = self.Pos;
			MovableMan:AddParticle(particle);
		end	
	end
	
	if self.InheritedRotAngleOffset ~= self.InheritedRotAngleTarget then
		self.InheritedRotAngleOffset = self.InheritedRotAngleOffset - (self.rotationSpeed * (self.InheritedRotAngleOffset - self.InheritedRotAngleTarget))
	end
	
	self.InheritedRotAngleTarget = 0;
	
end

function OnDetach(self)
	self.overheatLoopSound.Volume = 0;
	self.Overheated = false;
	self.bossChamberAnim = false;
	self:RemoveNumberValue("Busy");
	self.overheatStuckShell = false;
	self.InheritedRotAngleTarget = 0;
	self.rotationSpeed = 0.01;
end

function Destroy(self)
	self.overheatLoopSound:Stop(-1);
	self.overheatFixSound:Stop(-1);
end