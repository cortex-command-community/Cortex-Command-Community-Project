dofile("Base.rte/Constants.lua")

BrowncoatBossFunctions = {};

function BrowncoatBossFunctions.createVoiceSoundEffect(self, soundContainer, priority, canOverridePriority)
	if canOverridePriority == nil then
		canOverridePriority = false;
	end
	local usingPriority
	if canOverridePriority == false then
		usingPriority = priority - 1;
	else
		usingPriority = priority;
	end
	if self.Head and soundContainer ~= nil then
		if self.voiceSound then
			if self.voiceSound:IsBeingPlayed() then
				if self.lastPriority <= usingPriority then
					self.voiceSound:Stop();
					self.voiceSound = soundContainer;
					soundContainer:Play(self.Pos)
					self.lastPriority = priority;
					return true;
				end
			else
				self.voiceSound = soundContainer;
				soundContainer:Play(self.Pos)
				self.lastPriority = priority;
				return true;
			end
		else
			self.voiceSound = soundContainer;
			soundContainer:Play(self.Pos)
			self.lastPriority = priority;
			return true;
		end
	end
end

function BrowncoatBossFunctions.updateHealth(self)

	local healthTimerReady = self.healthUpdateTimer:IsPastSimMS(750);
	local wasInjured = self.Health < (self.oldHealth - self.PainThreshold) or self.Health <= 0;

	if (healthTimerReady or wasInjured) and not self.deathScripted then
	
		self.oldHealth = self.Health;
		self.healthUpdateTimer:Reset();
		if self.Health <= 0 then
			if not self.bossMode then
				BrowncoatBossFunctions.createVoiceSoundEffect(self, self.voiceSounds.Death, 11, true);
			else
				
				self.deathScripted = true;
				self.deathScriptedTimer:Reset();
				CameraMan:AddScreenShake(6, self.Pos);
				BrowncoatBossFunctions.createVoiceSoundEffect(self, self.voiceSounds.DeathScripted, 11, true);
				
				local woundTable = {};
				for wound in self:GetWounds() do
					table.insert(woundTable, wound);
				end
				
				print(#woundTable)
				
				if #woundTable > 0 then
					local fatalWoundedPart = woundTable[#woundTable]:GetParent();
					print(woundTable[#woundTable]);
					print(fatalWoundedPart);
					
					if fatalWoundedPart.UniqueID ~= self.Head.UniqueID and fatalWoundedPart.UniqueID ~= self.UniqueID then
						fatalWoundedPart.MissionCritical = false;
						fatalWoundedPart.BreakWound = CreateAEmitter("Browncoat Boss Scripted Death Break Wound", "Browncoats.rte");
						fatalWoundedPart.ParentBreakWound = CreateAEmitter("Browncoat Boss Scripted Death Break Wound", "Browncoats.rte");
						ToAttachable(fatalWoundedPart):RemoveFromParent(true, true);
					elseif fatalWoundedPart.UniqueID == self.Head.UniqueID then
						self.Head.Frame = 1;
					else -- it's the torso
						--self.Frame = 1;
					end
					
				end
				
			end
		elseif wasInjured then
			BrowncoatBossFunctions.createVoiceSoundEffect(self, self.voiceSounds.Pain, 2, true);
		end

	end
	
end

function BrowncoatBossFunctions.abilityShockwaveLanding(self)

	self.Jetpack.NegativeThrottleMultiplier = self.jumpPackDefaultNegativeMult;
	self.Jetpack.PositiveThrottleMultiplier = self.jumpPackDefaultPositiveMult;

	CameraMan:AddScreenShake(self.abilityShockwaveScreenShakeAmount, self.Pos);
	
	-- Breakable Ground Smoke
	for i = 1, 15 do	
		local effect = CreateMOSRotating("Breakable Smoke Ball Tiny", "Base.rte")
		effect.Pos = self.Pos;
		effect.Vel = self.Vel + Vector(math.random(-50, 50),math.random(90,150))
		effect.Lifetime = effect.Lifetime * RangeRand(0.5,2.0)
		effect.AirResistance = effect.AirResistance * RangeRand(0.5,0.8)
		MovableMan:AddParticle(effect)
	end
	
	for i = 1, 10 do	
		local effect = CreateMOSRotating("Breakable Smoke Ball", "Base.rte")
		effect.Pos = self.Pos;
		effect.Vel = self.Vel + Vector(math.random(-50, 50),math.random(90,150))
		effect.Lifetime = effect.Lifetime * RangeRand(0.5,2.0)
		effect.AirResistance = effect.AirResistance * RangeRand(0.5,0.8)
		MovableMan:AddParticle(effect)
	end

	-- Funkier Ground Smoke
	
	local groundPos = SceneMan:MovePointToGround(self.Pos, 1, 1);
	
	for i = 1, 25 do	
		local effect = CreateMOSParticle("Smoke Ball 1", "Base.rte")
		effect.Pos = groundPos;
		effect.Vel = self.Vel + Vector(math.random(-50, 50),math.random(-20,-30))
		effect.Lifetime = effect.Lifetime * RangeRand(0.5,2.0)
		effect.AirResistance = effect.AirResistance * RangeRand(0.5,0.8)
		MovableMan:AddParticle(effect)
	end
	
	for i = 1, 25 do	
		local effect = CreateMOSParticle("Tiny Smoke Ball 1", "Base.rte")
		effect.Pos = groundPos;
		effect.Vel = self.Vel + Vector(math.random(-50, 50),math.random(-20,-30))
		effect.Lifetime = effect.Lifetime * RangeRand(0.5,2.0)
		effect.AirResistance = effect.AirResistance * RangeRand(0.5,0.8)
		MovableMan:AddParticle(effect)
	end
	

	for mo in MovableMan:GetMOsInRadius(self.Pos, self.abilityShockwaveRange, -1, true) do
		if mo.Team ~= self.Team and mo.PinStrength == 0 and IsMOSRotating(mo) then
			local dist = SceneMan:ShortestDistance(self.Pos, mo.Pos, SceneMan.SceneWrapsX);
			local strSumCheck = SceneMan:CastStrengthSumRay(self.Pos, self.Pos + dist, 3, rte.airID);
			if strSumCheck < self.abilityShockwaveStrength then
				local massFactor = math.sqrt(1 + math.abs(mo.Mass));
				local distFactor = 1 + dist.Magnitude * 0.1;
				local forceVector =	dist:SetMagnitude((self.abilityShockwaveStrength - strSumCheck)/distFactor);
				if IsAttachable(mo) then
					--Diminish transferred impulses from attachables since we are likely already targeting its' parent
					forceVector = forceVector * math.abs(1 - ToAttachable(mo).JointStiffness);
				end
				mo.Vel = mo.Vel + forceVector/massFactor;
				mo.AngularVel = mo.AngularVel - forceVector.X/(massFactor + math.abs(mo.AngularVel));
				mo:AddImpulseForce(forceVector * massFactor, Vector());
				--Add some additional points of damage to actors
				if IsActor(mo) then
					local actor = ToActor(mo);
					local impulse = (forceVector.Magnitude * self.abilityShockwaveStrength/massFactor) - actor.ImpulseDamageThreshold;
					local damage = impulse/(actor.GibImpulseLimit * 0.1 + actor.Material.StructuralIntegrity * 10);
					actor.Health = damage > 0 and actor.Health - damage or actor.Health;
					actor.Status = (actor.Status == Actor.STABLE and damage > (actor.Health * 0.7)) and Actor.UNSTABLE or actor.Status;
				end
			end
		end
	end

end

function BrowncoatBossFunctions.JumpPack(self)

	self.jetpackEmitting = true;

	self.jumpPackCooldownTimer:Reset();
	
	self.isInAir = true;
	--self.jumpPackSound:Play(self.Pos);
	
	local offset = Vector(0, 2)
	
	local emitterA = CreateAEmitter("Browncoat Boss JumpPack Smoke Trail Medium")
	emitterA.Lifetime = 1300
	self.Jetpack:AddAttachable(emitterA);
	
	ToAttachable(emitterA).ParentOffset = offset
	
	local emitterB = CreateAEmitter("Browncoat Boss JumpPack Smoke Trail Heavy")
	emitterB.Lifetime = 400
	self.Jetpack:AddAttachable(emitterB);
	
	ToAttachable(emitterB).ParentOffset = offset
	
end