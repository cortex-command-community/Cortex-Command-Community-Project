-- This script incorporates Filipawn Industries code and the vanilla burstfire script together
-- There is likely better ways of doing a lot of this, potentially even standardizing it so it can be easily used more widely

-- Last worked on 29/08/2023

function OnFire(self)

	self.FireTimer:Reset();
	CameraMan:AddScreenShake(10, self.Pos);
	
end

function OnReload(self)

	self.reloadToSmoke = true;
	self.reloadSmokeTimer:Reset();
	
end

function Create(self)

	self.servoLoopSound = CreateSoundContainer("Coalition Bunker Cannon Servo Loop", "Coalition.rte");
	self.servoLoopSound.Volume = 0;
	self.servoLoopSound.Pitch = 1;
	self.servoLoopSound:Play(self.Pos);

	self.preSound = CreateSoundContainer("Coalition Bunker Cannon Pre", "Coalition.rte");
	
	self.explosiveShot = CreateAEmitter("Coalition Bunker Cannon Explosive Shot", "Coalition.rte");
	self.slugShot = CreateAEmitter("Coalition Bunker Cannon Slug Shot", "Coalition.rte");
	
	self.FireTimer = Timer();
	
	self.delayedFire = false
	self.delayedFireTimer = Timer();
	self.delayedFireTimeMS = 90;
	self.delayedFireEnabled = true;
	self.fireDelayTimer = Timer();
	self.activated = false;
	self.delayedFirstShot = true;
	
	self.reloadSmokeTimer = Timer();
	
	self.rotationSpeed = 0.10;
	self.smoothedRotAngle = self.RotAngle;
	self.InheritedRotAngleTarget = 0;

	self.shotsPerBurst = self:NumberValueExists("ShotsPerBurst") and self:GetNumberValue("ShotsPerBurst") or 3;
	self.coolDownDelay = 500;	
	
	if self:NumberValueExists("KeepUnflipped") then
		self.keepFlipped = true;
	else
		self.keepFlipped = false;
	end
	

end

function Update(self)

	self.servoLoopSound.Pos = self.Pos;

	self.HFlipped = self.keepFlipped;
	
	self.parent = IsActor(self:GetRootParent()) and ToActor(self:GetRootParent()) or nil;
	
	self.playerControlled = (self.parent and self.parent:IsPlayerControlled()) and true or false;
	
	-- reticule of actual aim line so the gun feels cannon-y rather than unresponsive
	
	if self.playerControlled and self.parent.SharpAimProgress > 0.13 then
		for i = 1, 24 do
			if i % 3 == 0 then
				local dotVec = Vector(i*self.FlipFactor, 0):RadRotate(self.RotAngle) + self.Pos + Vector((self.SharpLength + 15) * self.FlipFactor, 0):RadRotate(self.RotAngle)*self.parent.SharpAimProgress;
				PrimitiveMan:DrawLinePrimitive(dotVec, dotVec, 116, 2);
			end
		end
	end
	-- rotation smoothing, for a cannon-y feel:
	
	if self.smoothedRotAngle ~= self.RotAngle then
		self.smoothedRotAngle = self.smoothedRotAngle - (self.rotationSpeed * (self.smoothedRotAngle - self.RotAngle));
	end
	
	self.servoLoopSoundVolumeTarget = 0 + math.abs(self.smoothedRotAngle - self.RotAngle)
	self.servoLoopSound.Volume = self.servoLoopSound.Volume - (0.5 * (self.servoLoopSound.Volume - self.servoLoopSoundVolumeTarget));
	self.servoLoopSoundPitchTarget = 1 + math.abs(self.smoothedRotAngle - self.RotAngle)
	self.servoLoopSound.Pitch = self.servoLoopSound.Pitch - (0.1 * (self.servoLoopSound.Pitch - self.servoLoopSoundPitchTarget));
	
	self.InheritedRotAngleOffset = self.smoothedRotAngle - self.RotAngle;
	
	-- Mathemagical firing anim by filipex
	local f = math.max(1 - math.min((self.FireTimer.ElapsedSimTimeMS) / 200, 1), 0)
	self.Frame = math.floor(f * 8 + 0.55);
	
	if self:DoneReloading() or self:IsReloading() then
		self.fireDelayTimer:Reset();
		self.activated = false;
		self.delayedFire = false;
	end
	
	if self.delayedFire and self.delayedFireTimer:IsPastSimMS(self.delayedFireTimeMS) then
		self:Activate();
		self.delayedFire = false
		self.delayedFirstShot = false;
	end
	
	if self.Magazine then
		if self.coolDownTimer then
			if self.coolDownTimer:IsPastSimMS(self.coolDownDelay) and (self.playerControlled == false or self.triggerPulled == false) then
				self.coolDownTimer, self.shotCounter = nil;
				self.delayedFirstShot = true;
				-- it really throws off the AI if we don't deactivate here too, it'll only fire once
				-- i don't know why this is, it should just let it fire again
				self:Deactivate();
			else
				if not self:IsActivated() then
					self.triggerPulled = false;
				else	
					self:Deactivate();
				end
			end
		elseif self.shotCounter then

			self.triggerPulled = self:IsActivated();
				
			self:Activate();
			if self.FiredFrame then
			
				local shot = self.slugShot:Clone();
				shot.Pos = self.MuzzlePos;
				shot.Vel = self.Vel + Vector(160, 0):RadRotate(self.RotAngle);
				shot.Team = self.Team;
				shot.RotAngle = self.RotAngle;
				shot.HFlipped = self.HFlipped;
				MovableMan:AddParticle(shot);
				
				self.shotCounter = self.shotCounter + 1;
				if self.shotCounter >= self.shotsPerBurst then
					self.coolDownTimer = Timer();
					if self.RoundInMagCount == 0 then
						self:Reload();
					end
				end
			end
		elseif self.FiredFrame then
		
			self.triggerPulled = false;
		
			local shot = self.explosiveShot:Clone();
			shot.Pos = self.MuzzlePos;
			shot.Vel = self.Vel + Vector(160, 0):RadRotate(self.RotAngle);
			shot.Team = self.Team;
			shot.RotAngle = self.RotAngle;
			shot.HFlipped = self.HFlipped;
			MovableMan:AddParticle(shot);
			
			self.shotCounter = 1;
		end
	else
		self.coolDownTimer, self.shotCounter = nil;
	end	

	local fire = self:IsActivated() and self.RoundInMagCount > 0;

	if not self.shotCounter then
		if self.parent and self.delayedFirstShot == true then
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
			self.delayedFirstShot = true;
		end
	end
	
	if self:IsReloading() then
		-- manually timed according to sound
		if self.reloadToSmoke and self.reloadSmokeTimer:IsPastSimMS(900) then
			self.reloadToSmoke = false;
			
			for i = 1, 8 do
				local particle = CreateMOSParticle("Small Smoke Ball 1", "Base.rte");
				particle.GlobalAccScalar = 0.005
				particle.Lifetime = math.random(800, 2500);
				particle.Vel = self.Vel + Vector(math.random(-20, 20)/100, -math.random(-40, -30)/100);
				particle.Pos = self.Pos + Vector(-math.random(15, 17)*self.FlipFactor, math.random(-3, 3));
				MovableMan:AddParticle(particle);
			end
			
			for i = 1, 6 do
				local particle = CreateMOSParticle("Small Smoke Ball 1", "Base.rte");
				particle.GlobalAccScalar = 0.005
				particle.Lifetime = math.random(800, 2500);
				particle.Vel = self.Vel + Vector(math.random(-20, 20)/100, -math.random(-100, -30)/100);
				particle.Pos = self.Pos + Vector(-math.random(15, 17)*self.FlipFactor, math.random(-3, 3));
				MovableMan:AddParticle(particle);
			end	
			
		end
	end
	
end

function Destroy(self)

	self.servoLoopSound:Stop(-1);
	
end