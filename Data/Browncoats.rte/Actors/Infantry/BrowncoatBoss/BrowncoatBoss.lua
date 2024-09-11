require("Actors/Infantry/BrowncoatBoss/BrowncoatBossFunctions");
dofile("Base.rte/Constants.lua")

function Create(self)
	self.voiceSounds = {
	Pain = CreateSoundContainer("Browncoat Boss VO Pain", "Browncoats.rte"),
	Death = CreateSoundContainer("Browncoat Boss VO Death", "Browncoats.rte"),
	DeathScripted = CreateSoundContainer("Browncoat Boss VO DeathScripted", "Browncoats.rte"),
	JumpAttack = CreateSoundContainer("Browncoat Boss VO JumpAttack", "Browncoats.rte"),
	OilThrowTaunt = CreateSoundContainer("Browncoat Boss VO OilThrowTaunt", "Browncoats.rte"),
	ThrowGrunt = CreateSoundContainer("Browncoat Boss VO ThrowGrunt", "Browncoats.rte")}
	
	self.voiceSound = CreateSoundContainer("Browncoat Boss JumpPack", "Browncoats.rte");
	-- MEANINGLESS! this is just so we can do voiceSound.Pos without an if check first! it will be overwritten first actual VO play

	self.jumpPackSound = CreateSoundContainer("Browncoat Boss JumpPack", "Browncoats.rte");

	self.stepSound = CreateSoundContainer("Browncoat Boss Stride", "Browncoats.rte");	
	self.jumpSound = CreateSoundContainer("Browncoat Boss Jump", "Browncoats.rte");	
	self.landSound = CreateSoundContainer("Browncoat Boss Land", "Browncoats.rte");
	self.throwFoleySound = CreateSoundContainer("Browncoat Boss ThrowFoley", "Browncoats.rte");
	
	self.healthUpdateTimer = Timer();
	self.oldHealth = self.Health;
	
	self.PainThreshold = 7;
	
	-- leg Collision Detection system
	self.foot = 0;
    self.feetContact = {false, false}
    self.feetTimers = {Timer(), Timer()}
	self.footstepTime = 100 -- 2 Timers to avoid noise	
	
	-- Custom Jumping
	self.jumpStrength = 4
	self.isJumping = false
	self.jumpTimer = Timer();
	self.jumpDelay = 500;
	self.jumpStop = Timer();

	self.jumpPackDefaultNegativeMult = 0.7;
	self.jumpPackDefaultPositiveMult = 1.3;
	self.jumpPackCooldownTimer = Timer();
	self.jumpPackCooldownTime = 0;
	
	self.altitude = 0;
	self.isInAir = false;
	
	self.moveSoundTimer = Timer();
	
	if self.PresetName == "Browncoat Boss Scripted" then
		self.bossMode = true;
	end
end

function OnStride(self)
	if self.BGFoot and self.FGFoot then
		local startPos = self.foot == 0 and self.BGFoot.Pos or self.FGFoot.Pos
		self.foot = (self.foot + 1) % 2
		
		local pos = Vector(0, 0);
		SceneMan:CastObstacleRay(startPos, Vector(0, 9), pos, Vector(0, 0), self.ID, self.Team, 0, 3);				
		local terrPixel = SceneMan:GetTerrMatter(pos.X, pos.Y)
		
		if terrPixel ~= 0 then -- 0 = air
			self.stepSound:Play(self.Pos);
		end
	elseif self.BGFoot then
		local startPos = self.BGFoot.Pos
		
		local pos = Vector(0, 0);
		SceneMan:CastObstacleRay(startPos, Vector(0, 9), pos, Vector(0, 0), self.ID, self.Team, 0, 3);				
		local terrPixel = SceneMan:GetTerrMatter(pos.X, pos.Y)
		
		if terrPixel ~= 0 then -- 0 = air
			self.stepSound:Play(self.Pos);
		end
	elseif self.FGFoot then
		local startPos = self.FGFoot.Pos
		
		local pos = Vector(0, 0);
		SceneMan:CastObstacleRay(startPos, Vector(0, 9), pos, Vector(0, 0), self.ID, self.Team, 0, 3);				
		local terrPixel = SceneMan:GetTerrMatter(pos.X, pos.Y)
		
		if terrPixel ~= 0 then -- 0 = air
			self.stepSound:Play(self.Pos);
		end
	end
end

function OnMessage(self, message, context)
	if message == "AI_IsFlying" then
		if not self:IsPlayerControlled() then
			if context == true and self.isInAir == false then
				self.isInAir = true;
			elseif context == false and self.isInAir == true then
				self.isInAir = false;
			end
		end
	end
end

function Update(self)
	self.voiceSound.Pos = self.Pos;

	self.controller = self:GetController();
	
	BrowncoatBossFunctions.updateHealth(self);

	-- Leg Collision Detection system
    --local i = 0
	if self:IsPlayerControlled() then -- AI doesn't update its own foot checking when playercontrolled so we have to do it
		if self.Vel.Y > 5 then
			self.isInAir = true;
		else
			self.isInAir = false;
		end

		for i = 1, 2 do
			--local foot = self.feet[i]
			local foot = nil
			--local leg = self.legs[i]
			if i == 1 then
				foot = self.FGFoot 
			else
				foot = self.BGFoot 
			end

			--if foot ~= nil and leg ~= nil and leg.ID ~= rte.NoMOID then
			if foot ~= nil then
				local footPos = foot.Pos				
				local mat = nil
				local pixelPos = footPos + Vector(0, 4)
				self.footPixel = SceneMan:GetTerrMatter(pixelPos.X, pixelPos.Y)
				--PrimitiveMan:DrawLinePrimitive(pixelPos, pixelPos, 13)
				if self.footPixel ~= 0 then
					mat = SceneMan:GetMaterialFromID(self.footPixel)
				--	PrimitiveMan:DrawLinePrimitive(pixelPos, pixelPos, 162);
				--else
				--	PrimitiveMan:DrawLinePrimitive(pixelPos, pixelPos, 13);
				end
				
				local movement = (self.controller:IsState(Controller.MOVE_LEFT) == true or self.controller:IsState(Controller.MOVE_RIGHT) == true or self.Vel.Magnitude > 3)
				if mat ~= nil then
					--PrimitiveMan:DrawTextPrimitive(footPos, mat.PresetName, true, 0);
					if self.feetContact[i] == false then
						self.feetContact[i] = true
						if self.feetTimers[i]:IsPastSimMS(self.footstepTime) and movement then																	
							self.feetTimers[i]:Reset()
						end
					end
				else
					if self.feetContact[i] == true then
						self.feetContact[i] = false
						if self.feetTimers[i]:IsPastSimMS(self.footstepTime) and movement then
							self.feetTimers[i]:Reset()
						end
					end
				end
			end
		end
	end
	
	-- Jumppack custom fx, for extra control actor-side
	
	--local jumpPackTrigger = self.Jetpack and self.Jetpack:IsEmitting() and not self.jetpackEmitting;
	
	--if jumpPackTrigger then
	--	BrowncoatBossFunctions.JumpPack(self)
	--end
	
	--if not self.Jetpack or not self.Jetpack:IsEmitting() then
	--	self.jetpackEmitting = false;
	--end

	-- Jump to make this guy more bearable movement-wise
	-- Also for extra cool boss ability
	
	local jump = (self.controller:IsState(Controller.BODY_JUMPSTART) == true and
		self.controller:IsState(Controller.BODY_CROUCH) == false and
		self.jumpTimer:IsPastSimMS(self.jumpDelay) and
		not self.isJumping and
		not jumpPackTrigger);
	
	if jump or self.abilityShockwaveTrigger then
		if (self:IsPlayerControlled() and self.feetContact[1] == true or self.feetContact[2] == true) or self.isInAir == false then
			local jumpVec = Vector(0,-self.jumpStrength)
			
			if self.abilityShockwaveTrigger then
			
				jumpVec = jumpVec * 2
			
				self.abilityShockwaveTrigger = false;
				self.abilityShockwaveTimer:Reset();
				self.abilityShockwaveOngoing = true;
				BrowncoatBossFunctions.createVoiceSoundEffect(self, self.voiceSounds.JumpAttack, 10, true);
			end			
			
			local jumpWalkX = 3
			if self.controller:IsState(Controller.MOVE_LEFT) == true then
				jumpVec.X = -jumpWalkX
			elseif self.controller:IsState(Controller.MOVE_RIGHT) == true then
				jumpVec.X = jumpWalkX
			end

			self.jumpSound:Play(self.Pos);
			if math.abs(self.Vel.X) > jumpWalkX * 2.0 then
				self.Vel = Vector(self.Vel.X, self.Vel.Y + jumpVec.Y)
			else
				self.Vel = Vector(self.Vel.X + jumpVec.X, self.Vel.Y + jumpVec.Y)
			end

			self.isJumping = true
			self.jumpTimer:Reset()
			self.jumpStop:Reset()
		end
	elseif self.isJumping or self.isInAir then
		if (self:IsPlayerControlled() and self.feetContact[1] == true or self.feetContact[2] == true) and self.jumpStop:IsPastSimMS(100) then
			self.isJumping = false
			self.isInAir = false;
			if (self.Vel.Y > 0 and self.moveSoundTimer:IsPastSimMS(500)) or self.abilityShockwaveOngoing == true then
				if self.abilityShockwaveOngoing == true then
					self.abilityShockwaveOngoing = false;
					self.abilityShockwaveWhooshSound:Stop(-1);
					self.voiceSound:FadeOut(90);
					
					self.abilityShockwaveLandSound:Play(self.Pos);
					
					BrowncoatBossFunctions.abilityShockwaveLanding(self);
				else
					self.landSound:Play(self.Pos);
				end
				
				self.moveSoundTimer:Reset();			
			end
		end
	end
	
	-- Throw Foley
	if self.EquippedItem and IsTDExplosive(self.EquippedItem) and (self.EquippedItem:IsActivated() or self.controller:IsState(Controller.PRIMARY_ACTION)) then
		self.toThrowFoley = true;
	elseif self.toThrowFoley then
		self.toThrowFoley = false;
		BrowncoatBossFunctions.createVoiceSoundEffect(self, self.voiceSounds.ThrowGrunt, 3, false);
		self.throwFoleySound:Play(self.Pos);
	end
end