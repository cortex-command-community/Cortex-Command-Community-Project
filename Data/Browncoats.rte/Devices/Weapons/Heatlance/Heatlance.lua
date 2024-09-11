function Create(self)
	self.igniteSound = CreateSoundContainer("Ignite FT-200", "Browncoat.rte");
	
	self.flameSwingLoopSound = CreateSoundContainer("Flame Swing Loop FT-200", "Browncoat.rte");
	
	self.plumeLoopSound = CreateSoundContainer("Plume Loop FT-200", "Browncoat.rte");
	self.plumeEndSound = CreateSoundContainer("Plume End FT-200", "Browncoat.rte");
	
	self.plumeCheckTimer = Timer();
	self.plumeCheckTime = 300;
	self.plumePosDifference = Vector();
	
	self.lastRotAngle = 0;
	
	self.delayedFire = false;
	self.delayedFireTimer = Timer();
	self.delayedFireTimeMS = 100;
	self.delayedFireEnabled = true;
	self.fireDelayTimer = Timer();
	self.activated = false;
	self.delayedFirstShot = true;
end

function ThreadedUpdate(self)
	if self.Magazine then
		local parent = self:GetRootParent();
		if parent and IsActor(parent) then
			parent = ToActor(parent);
			local parentDimensions = Vector(ToMOSprite(parent):GetSpriteWidth(), ToMOSprite(parent):GetSpriteHeight());
			local magDimensions = Vector(self.Magazine:GetSpriteWidth(), self.Magazine:GetSpriteHeight());
			self.Magazine.Pos = parent.Pos + Vector(-(parentDimensions.X + magDimensions.X) * 0.4 * parent.FlipFactor, 1 - (parentDimensions.Y + magDimensions.Y) * 0.2):RadRotate(parent.RotAngle);
			self.Magazine.RotAngle = parent.RotAngle;
		end
	end
	
	if self.delayedFire and self.delayedFireTimer:IsPastSimMS(self.delayedFireTimeMS) then
		self:Activate();
		self.flameSwingLoopSound:Play(self.Pos);
		self.plumeLoopSound.Volume = 0;
		self.plumeLoopSound:Play(self.Pos);
		local finalVec = Vector();
		local flameVector = Vector(250 * self.FlipFactor, 0):RadRotate(self.RotAngle);
		SceneMan:CastObstacleRay(self.Pos, flameVector, Vector(), finalVec, self.ID, self.Team, 128, 7);
		self.plumePosDifference = SceneMan:ShortestDistance(self.plumeLoopSound.Pos, finalVec, false);
		self.delayedFire = false;
		self.delayedFirstShot = false;
	end

	local fire = self:IsActivated() and self.RoundInMagCount > 0;
	
	if self.delayedFirstShot == true then
		if self.RoundInMagCount > 0 then
			self:Deactivate()
		end
		
		--if self.parent:GetController():IsState(Controller.WEAPON_FIRE) and not self:IsReloading() then
		if fire and not self:IsReloading() then
			if not self.Magazine or self.RoundInMagCount < 1 then
				--self:Reload()
				self:Activate();
			elseif not self.activated and not self.delayedFire and self.fireDelayTimer:IsPastSimMS(1 / (self.RateOfFire / 60) * 1000) then
				self.activated = true;
				
				self.igniteSound:Play(self.Pos);
				
				self.fireDelayTimer:Reset();
				
				self.delayedFire = true;
				self.delayedFireTimer:Reset();
			end
		else
			if self.activated then
				self.activated = false
			end
		end
	elseif fire == false then
		self.plumeEndSound:Play(self.plumeLoopSound.Pos);
		self.igniteSound:FadeOut(500);
		self.firstShot = true;
		self.delayedFirstShot = true;
	end
	
	if fire then
		self.flameSwingLoopSound.Pos = self.plumeLoopSound.Pos;
	
		local swingFactor = math.abs(self.RotAngle - self.lastRotAngle) * 15;
		
		if self.flameSwingLoopSound.Volume < swingFactor then
			self.flameSwingLoopSound.Volume = math.min(1, self.flameSwingLoopSound.Volume + TimerMan.DeltaTimeSecs * 5);
		end
		
		if self.flameSwingLoopSound.Volume > swingFactor then
			self.flameSwingLoopSound.Volume = math.min(1, self.flameSwingLoopSound.Volume - TimerMan.DeltaTimeSecs * 2);
		end
	
		--PrimitiveMan:DrawCirclePrimitive(self.Pos, self.flameSwingLoopSound.Volume * 100, 50);
		
		self.lastRotAngle = self.RotAngle;
	
		if self.plumeCheckTimer:IsPastSimMS(self.plumeCheckTime) then
			self.plumeCheckTimer:Reset();
			
			local finalVec = Vector();
			local flameVector = Vector(270 * self.FlipFactor, 0):RadRotate(self.RotAngle);
			SceneMan:CastObstacleRay(self.Pos, flameVector, Vector(), finalVec, self.ID, self.Team, 128, 7);
			self.plumePosDifference = SceneMan:ShortestDistance(self.plumeLoopSound.Pos, finalVec, false);
		end

		if self.plumePosDifference.Magnitude > 5 then
			self.plumeLoopSound.Pos = self.plumeLoopSound.Pos + (self.plumePosDifference*(TimerMan.DeltaTimeSecs*2));
		end

		if self.plumeLoopSound.Volume < 1 then
			self.plumeLoopSound.Volume = math.min(1, self.plumeLoopSound.Volume + TimerMan.DeltaTimeSecs * 7);
			self.plumeEndSound.Volume = self.plumeLoopSound.Volume;
		end
		--PrimitiveMan:DrawCirclePrimitive(self.plumeLoopSound.Pos, 10, 50);
	else
		if self.plumeLoopSound.Volume > 0 then
			self.plumeLoopSound.Volume = math.max(0, self.plumeLoopSound.Volume - TimerMan.DeltaTimeSecs * 10);
		end

		if self.flameSwingLoopSound.Volume > 0 then
			self.flameSwingLoopSound.Volume = math.max(0, self.flameSwingLoopSound.Volume - TimerMan.DeltaTimeSecs * 2);
		end
	end
end

function OnDestroy(self)
	self.flameSwingLoopSound.Volume = 0;
	self.plumeLoopSound.Volume = 0;
end

function OnDetach(self)
	self.flameSwingLoopSound.Volume = 0;
	self.plumeLoopSound.Volume = 0;
end