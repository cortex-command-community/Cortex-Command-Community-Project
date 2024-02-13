-- This script incorporates Filipawn Industries code and the vanilla burstfire script together
-- There is likely better ways of doing a lot of this, potentially even standardizing it so it can be easily used more widely

function OnFire(self)
	CameraMan:AddScreenShake(7, self.Pos);
	
	local shot = self.Shot:Clone();
	shot.Pos = self.MuzzlePos;
	shot.Vel = self.Vel + Vector(160, 0):RadRotate(self.RotAngle);
	shot.Team = self.Team;
	shot.RotAngle = self.RotAngle;
	shot.HFlipped = self.HFlipped;
	MovableMan:AddParticle(shot);
	
	self.animTimer:Reset();
	self.firingAnim = true;
end

function OnReload(self)
	self.reloadToSmoke = true;
	self.animTimer:Reset();
	
	if self.currentBaseFrame ~= 20 then
		self.oldFrame = self.Frame;
	end
end

function Create(self)
	-- self.servoLoopSound = CreateSoundContainer("Coalition Bunker Cannon Servo Loop", "Coalition.rte");
	-- self.servoLoopSound.Volume = 0;
	-- self.servoLoopSound.Pitch = 1;
	-- self.servoLoopSound:Play(self.Pos);
	
	self.Shot = CreateAEmitter("Browncoat AA-50 Shot", "Browncoats.rte");

	self.firingAnim = false;
	self.animTimer = Timer();
	self.firingAnimTime = (1 / (self.RateOfFire / 60) * 1000) - 30;	-- -30 for some buffer
	self.currentBaseFrame = 0;
	
	self.currentBarrel = 0;
	
	self.topMuzzleOffset = Vector(55, -8);
	self.bottomMuzzleOffset = Vector(55, 6);
	
	self.MuzzleOffset = self.topMuzzleOffset;
	
	for att in self.Attachables do
		if string.find(att.PresetName, "Top") then	
			self.topBarrel = ToAttachable(att);
		elseif string.find(att.PresetName, "Bottom") then
			self.bottomBarrel = ToAttachable(att);
		end
	end
	
	self.reloadSmokeTimer = Timer();
	
	self.rotationSpeed = 0.10;
	self.smoothedRotAngle = self.RotAngle;
	self.InheritedRotAngleTarget = 0;
end

function Update(self)
	--self.servoLoopSound.Pos = self.Pos;

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
	
	-- self.servoLoopSoundVolumeTarget = 0 + math.abs(self.smoothedRotAngle - self.RotAngle)
	-- self.servoLoopSound.Volume = self.servoLoopSound.Volume - (0.5 * (self.servoLoopSound.Volume - self.servoLoopSoundVolumeTarget));
	-- self.servoLoopSoundPitchTarget = 1 + math.abs(self.smoothedRotAngle - self.RotAngle)
	-- self.servoLoopSound.Pitch = self.servoLoopSound.Pitch - (0.1 * (self.servoLoopSound.Pitch - self.servoLoopSoundPitchTarget));
	
	self.InheritedRotAngleOffset = self.smoothedRotAngle - self.RotAngle;
	
	if self:DoneReloading() then
		self.currentBaseFrame = 0;
		self.Frame = 0;
	end
	
	if self.firingAnim then
		self:Deactivate();
	
		local progress = math.min(1, self.animTimer.ElapsedSimTimeMS / self.firingAnimTime);
		local frameNum = math.floor(4 * progress);
		self.Frame = self.currentBaseFrame + frameNum;
		
		local barrel = self.currentBarrel == 0 and self.topBarrel or self.bottomBarrel;
		local jointOffsetX = 10 * math.sin(progress * math.pi);
		barrel.JointOffset = Vector(jointOffsetX, 0);
		
		if progress == 1 then
			self.MuzzleOffset = self.currentBarrel == 0 and self.bottomMuzzleOffset or self.topMuzzleOffset;
			barrel.JointOffset = Vector();
			self.currentBarrel = (self.currentBarrel + 1) % 2;
			self.firingAnim = false;
			
			-- surely this can be done better...
			if not self:IsReloading() then
				if self.RoundInMagCount == 1 then
					self.currentBaseFrame = 20;
				elseif self.RoundInMagCount == 2 then
					self.currentBaseFrame = 16;
				elseif self.RoundInMagCount == 3 then
					self.currentBaseFrame = 12;
				elseif self.RoundInMagCount == 4 then
					self.currentBaseFrame = 8;
				elseif self.RoundInMagCount == 5 then
					self.currentBaseFrame = 4;
				end
				
				self.Frame = self.currentBaseFrame;
			end
		end
	end
				
	if self:IsReloading() then
		-- manually timed
		
		if self.currentBaseFrame ~= 20 then
			local progress = math.min(1, self.animTimer.ElapsedSimTimeMS / (self.firingAnimTime*3));
			local frameNum = math.floor((20 - self.oldFrame) * progress);
			self.Frame = self.oldFrame + frameNum;
			if self.Frame == 20 then
				self.currentBaseFrame = 20;
				self.oldFrame = nil;
				self.Frame = 20;
			end
		end
		
		if self.animTimer:IsPastSimMS(self.ReloadTime - 1000) then
			local progress = math.min(1, (self.animTimer.ElapsedSimTimeMS - (self.ReloadTime - 1000)) / 1000);
			local frameNum = math.floor(17 * progress);
			self.Frame = self.currentBaseFrame + frameNum;
		end
		
		if self.reloadToSmoke and self.animTimer:IsPastSimMS(900) then
			self.reloadToSmoke = false;
			
			for i = 1, 8 do
				local particle = CreateMOSParticle("Small Smoke Ball 1", "Base.rte");
				particle.GlobalAccScalar = 0.005
				particle.Lifetime = math.random(800, 2500);
				particle.Vel = self.Vel + Vector(math.random(-20, 20)/100, -math.random(-40, -30)/100);
				particle.Pos = self.Pos
				MovableMan:AddParticle(particle);
			end
			
			for i = 1, 6 do
				local particle = CreateMOSParticle("Small Smoke Ball 1", "Base.rte");
				particle.GlobalAccScalar = 0.005
				particle.Lifetime = math.random(800, 2500);
				particle.Vel = self.Vel + Vector(math.random(-20, 20)/100, -math.random(-100, -30)/100);
				particle.Pos = self.Pos
				MovableMan:AddParticle(particle);
			end	
		end
	end
				
	
end

function Destroy(self)
	self.servoLoopSound:Stop(-1);
end