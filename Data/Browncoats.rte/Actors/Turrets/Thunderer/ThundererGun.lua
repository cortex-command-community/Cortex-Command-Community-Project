function OnFire(self)
	CameraMan:AddScreenShake(12, self.Pos);
	
	local shot = self.Shot:Clone();
	shot.Pos = self.MuzzlePos;
	shot.Vel = self.Vel + Vector(160, 0):RadRotate(self.RotAngle);
	shot.Team = self.Team;
	shot.RotAngle = self.RotAngle;
	shot.HFlipped = self.HFlipped;
	MovableMan:AddParticle(shot);
	
	self.animTimer:Reset();
	self.firingAnim = true;
	
	local casing = self.Casing:Clone();
	casing.Pos = self.Pos + (self.currentBarrel == 0 and self.bottomEjectorOffset or self.topEjectorOffset);
	casing.Vel = self.Vel + Vector(0, self.currentBarrel == 0 and 5 or -5):RadRotate(self.RotAngle);
	casing.Team = self.Team;
	casing.RotAngle = self.RotAngle;
	casing.HFlipped = self.HFlipped;
	casing.AngularVel = self.currentBarrel == 0 and math.random(-3, -5) or math.random(3, 5);
	MovableMan:AddParticle(casing);
	
	-- Set it up for next time, so we're sitting on the right MuzzleOffset rather than the wrong one up until the point of fire
	self.MuzzleOffset = self.currentBarrel == 1 and self.bottomMuzzleOffset or self.topMuzzleOffset;
end

function OnReload(self)
	self.reloadToSmoke = true;
	self.animTimer:Reset();
end

function Create(self)
	self.servoStartSound = CreateSoundContainer("Large Generic Servo Start", "Base.rte");
	self.servoStartSound.Volume = 0.25;
	self.servoLoopSound = CreateSoundContainer("Large Generic Servo Loop", "Base.rte");
	self.servoLoopSound.Volume = 0;
	self.servoLoopSound.Pitch = 1;
	self.servoLoopSound:Play(self.Pos);
	self.servoEndSound = CreateSoundContainer("Large Generic Servo End", "Base.rte");
	self.servoEndSound.Volume = 0.125;
	
	self.servoMoving = false;
	
	self.Casing= CreateAEmitter("Casing Browncoat AA-50", "Browncoats.rte");
	self.Shot = CreateAEmitter("Shot Browncoat AA-50", "Browncoats.rte");

	self.firingAnim = false;
	self.animTimer = Timer();
	self.firingAnimTime = (1 / (self.RateOfFire / 60) * 1000) - 30;	-- -30 for some buffer
	self.currentBaseFrame = 0;
	
	self.currentBarrel = 0;
	
	self.topMuzzleOffset = Vector(42, -4);
	self.bottomMuzzleOffset = Vector(42, 10);
	
	self.topEjectorOffset = Vector(-5, -4);
	self.bottomEjectorOffset = Vector(-5, 10);
	
	self.MuzzleOffset = self.bottomMuzzleOffset;
	
	for att in self.Attachables do
		if string.find(att.PresetName, "Barrel Top") then	
			self.topBarrel = ToAttachable(att);
		elseif string.find(att.PresetName, "Barrel Bottom") then
			self.bottomBarrel = ToAttachable(att);
		end
		if string.find(att.PresetName, "Ejector Top") then	
			self.topEjector = ToAttachable(att);
		elseif string.find(att.PresetName, "Ejector Bottom") then
			self.bottomEjector = ToAttachable(att);
		end
	end
	
	self.reloadSmokeTimer = Timer();
	
	self.rotationSpeed = 0.08;
	self.rotAngleDeviation = 0;
	
	if self.HFlipped then
		self.RotAngle = math.pi;
	end
	
	self.LastHFlipped = self.HFlipped;
	self.LastRotAngle = self.RotAngle;
end

function Update(self)
	self.servoLoopSound.Pos = self.Pos;
	
    if self.LastHFlipped ~= nil then
        if self.LastHFlipped ~= self.HFlipped then
            self.LastHFlipped = self.HFlipped
            self.rotAngleDeviation = 0;
        end
    end
	
	self.parent = IsActor(self:GetRootParent()) and ToActor(self:GetRootParent()) or nil;
	self.playerControlled = (self.parent and self.parent:IsPlayerControlled()) and true or false;
	
	-- Rotation smoothing related stuff
	if self.parent then	
		local actingRotAngle = self.parent:GetAimAngle(true)
		local aimAngle = self.parent:GetAimAngle(false) * self.FlipFactor;
		
		-- Reticule
		if self.playerControlled and self.parent.SharpAimProgress > 0.13 then
			for i = 1, 24 do
				if i % 3 == 0 then
					local dotVec = Vector(i*self.FlipFactor, 0):RadRotate(aimAngle) + self.Pos + Vector((self.SharpLength + 15) * self.FlipFactor, 0):RadRotate(aimAngle)*self.parent.SharpAimProgress;
					PrimitiveMan:DrawLinePrimitive(dotVec, dotVec, 116, 2);
				end
			end
		end
		
		self.rotAngleDeviation = self.rotAngleDeviation + (self.LastRotAngle - actingRotAngle);

		if self.rotAngleDeviation ~= 0 then
			self.rotAngleDeviation = self.rotAngleDeviation - (self.rotAngleDeviation * self.rotationSpeed);
			if math.abs(self.rotAngleDeviation) < 0.001 then
				self.rotAngleDeviation = 0;
			end
		end
		
		self.InheritedRotAngleOffset = self.rotAngleDeviation * self.FlipFactor;
		self.LastRotAngle = actingRotAngle;
		
		self.servoLoopSoundVolumeTarget = 0 + math.abs(self.rotAngleDeviation)
		self.servoLoopSound.Volume = self.servoLoopSound.Volume - (0.5 * (self.servoLoopSound.Volume - self.servoLoopSoundVolumeTarget));
		self.servoLoopSoundPitchTarget = 1 + math.abs(self.rotAngleDeviation)
		self.servoLoopSound.Pitch = self.servoLoopSound.Pitch - (0.1 * (self.servoLoopSound.Pitch - self.servoLoopSoundPitchTarget));
		
		if self.servoMoving then
			if self.servoLoopSoundVolumeTarget < 0.10 then
				self.servoStartSound:Stop(-1);
				self.servoEndSound:Play(self.Pos);
				self.servoMoving = false;
			end
		else
			if self.servoLoopSoundVolumeTarget > 0.20 then
				self.servoEndSound:Stop(-1);
				self.servoStartSound:Play(self.Pos);
				self.servoMoving = true;
			end
		end
	else
		self.servoLoopSound.Volume = 0;
	end
	
	if self:DoneReloading() then
		self.currentBaseFrame = 0;
		self.Frame = 0;
		self.currentBarrel = 0;
		self.MuzzleOffset = self.bottomMuzzleOffset;
	end
				
	if self:IsReloading() then
		-- manually timed
		
		if self.currentBaseFrame ~= 22 then
			local progress = math.min(1, self.animTimer.ElapsedSimTimeMS / (self.firingAnimTime*3));
			local frameNum = self.currentBaseFrame + math.floor((22 - self.currentBaseFrame) * progress);
			self.Frame = frameNum;
			if self.Frame == 22 then
				self.currentBaseFrame = 22;
			end
		end
		
		if self.animTimer:IsPastSimMS(self.ReloadTime - 1000) then
			local progress = math.min(1, (self.animTimer.ElapsedSimTimeMS - (self.ReloadTime - 1000)) / 1000);
			local frameNum = math.floor(21 * progress);
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
	elseif self.firingAnim then
		self:Deactivate();
	
		local progress = math.min(1, self.animTimer.ElapsedSimTimeMS / self.firingAnimTime);
		local frameNum = math.floor(4 * progress);
		self.Frame = self.currentBaseFrame + frameNum;
		
		local barrel = self.currentBarrel == 0 and self.bottomBarrel or self.topBarrel;
		local ejector = self.currentBarrel == 0 and self.bottomEjector or self.topEjector;
		local jointOffsetX = 10 * math.sin(progress * math.pi);
		barrel.JointOffset = Vector(jointOffsetX, 0);
		ejector.JointOffset = Vector(jointOffsetX, 0);
		if progress == 1 then
			-- surely this can be done better...
			if not self:IsReloading() then
				if self.currentBarrel == 0 then
					self.currentBaseFrame = 4;
				else
					self.currentBaseFrame = 0;
				end
				if self.RoundInMagCount == 1 then
					self.currentBaseFrame = 24;
				elseif self.RoundInMagCount == 2 then
					self.currentBaseFrame = 20;
				elseif self.RoundInMagCount == 3 then
					self.currentBaseFrame = 16;
				elseif self.RoundInMagCount == 4 then
					self.currentBaseFrame = 12;
				elseif self.RoundInMagCount == 4 then
					self.currentBaseFrame = 8;
				end			
				self.Frame = self.currentBaseFrame;
			end
			barrel.JointOffset = Vector();
			self.currentBarrel = (self.currentBarrel + 1) % 2;
			self.firingAnim = false;			
		end
	end
end

function Destroy(self)
	self.servoLoopSound:Stop(-1);
end