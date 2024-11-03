require("Actors/Infantry/BrowncoatBoss/BrowncoatBossFunctions");
dofile("Base.rte/Constants.lua")

function Create(self)
	self.voiceSounds = {
	LightPain = CreateSoundContainer("Browncoat Boss VO Light Pain", "Browncoats.rte"),
	MediumPain = CreateSoundContainer("Browncoat Boss VO Medium Pain", "Browncoats.rte"),
	HeavyPain = CreateSoundContainer("Browncoat Boss VO Heavy Pain", "Browncoats.rte"),
	Death = CreateSoundContainer("Browncoat Boss VO Death", "Browncoats.rte"),
	Throw = CreateSoundContainer("Browncoat Boss VO Throw", "Browncoats.rte"),
	Jump = CreateSoundContainer("Browncoat Boss VO Jump", "Browncoats.rte"),
	Land = CreateSoundContainer("Browncoat Boss VO Land", "Browncoats.rte"),
	SquadLead = CreateSoundContainer("Browncoat Boss VO Squad Lead", "Browncoats.rte"),
	Taunt = CreateSoundContainer("Browncoat Boss VO Taunt", "Browncoats.rte"),
	MonologueOutro = CreateSoundContainer("Browncoat Boss VO Monologue Outro", "Browncoats.rte"),
	DeathScripted = CreateSoundContainer("Browncoat Boss VO Death Scripted", "Browncoats.rte"),
	JumpAttack = CreateSoundContainer("Browncoat Boss VO Ability Jump Attack", "Browncoats.rte"),
	OilThrowTaunt = CreateSoundContainer("Browncoat Boss VO Ability Oil Throw", "Browncoats.rte")}
	
	self.voiceSound = CreateSoundContainer("Browncoat Boss Jump Pack", "Browncoats.rte");
	-- MEANINGLESS! this is just so we can do voiceSound.Pos without an if check first! it will be overwritten first actual VO play

	self.jumpPackSound = CreateSoundContainer("Browncoat Boss Jump Pack", "Browncoats.rte");

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
	
	self.squadLeadVoiceLineTimer = Timer();
	self.squadLeadVoiceLineDelay = 15000;
	
	if self.PresetName == "Browncoat Boss Scripted" then
		self.bossMode = true;
		self.tauntVoiceLineTimer = Timer();
		self.tauntVoiceLineDelay = 45000;
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
	if self:IsPlayerControlled() then -- AI doesn't update its own foot checking when playercontrolled so we have to do it
		if self.Vel.Y > 5 then
			self.isInAir = true;
		else
			self.isInAir = false;
		end

		for i = 1, 2 do
			local foot = nil
			if i == 1 then
				foot = self.FGFoot 
			else
				foot = self.BGFoot 
			end

			if foot ~= nil then
				local footPos = foot.Pos				
				local mat = nil
				local pixelPos = footPos + Vector(0, 4)
				self.footPixel = SceneMan:GetTerrMatter(pixelPos.X, pixelPos.Y)
				if self.footPixel ~= 0 then
					mat = SceneMan:GetMaterialFromID(self.footPixel)
				end
				
				local movement = (self.controller:IsState(Controller.MOVE_LEFT) == true or self.controller:IsState(Controller.MOVE_RIGHT) == true or self.Vel.Magnitude > 3)
				if mat ~= nil then
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

	-- Jump to make this guy more bearable movement-wise
	-- Also for extra cool boss ability
	
	local jump = (self.controller:IsState(Controller.BODY_JUMPSTART) == true and
		self.controller:IsState(Controller.BODY_PRONE) == false and
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
			
			if math.random(0, 100) < 20 then
				BrowncoatBossFunctions.createVoiceSoundEffect(self, self.voiceSounds.Jump, 2, false);
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
					
					-- it's stupid that this is here in force, but the rest of the game has nothing like this yet and so it isn't standardized.
					local terrainIDs = {
					[12] = "Concrete",
					[164] = "Concrete",
					[176] = "Concrete",
					[177] = "Concrete",
					[9] = "Dirt",
					[10] = "Dirt",
					[11] = "Dirt",
					[128] = "Dirt",
					[6] = "Dirt",
					[8] = "Dirt",
					[178] = "Metal",
					[179] = "Metal",
					[180] = "Metal",
					[181] = "Metal",
					[182] = "Metal"};
					
					local hitPos = Vector(0, 0);
					local ray = SceneMan:CastObstacleRay(self.Pos, Vector(0, 50), hitPos, Vector(0, 0), self.ID, self.Team, 0, 3);	
					if ray ~= -1 then
						terrainID = SceneMan:GetTerrMatter(hitPos.X, hitPos.Y)
						
						if terrainID ~= -1 then
							if terrainIDs[terrainID] then
								if self.abilityShockwaveTerrainSounds[terrainIDs[terrainID]] ~= nil then
									self.abilityShockwaveTerrainSounds[terrainIDs[terrainID]]:Play(self.Pos);
								end
							end	
						end
					end
					
					self.abilityShockwaveLandSound:Play(self.Pos);
					
					BrowncoatBossFunctions.abilityShockwaveLanding(self);
				else
					self.landSound:Play(self.Pos);
					if math.random(0, 100) < 20 then
						BrowncoatBossFunctions.createVoiceSoundEffect(self, self.voiceSounds.Land, 2, false);
					end
				end
				
				
				self.moveSoundTimer:Reset();			
			end
		end
	end
	
	-- Throw foley
	if self.EquippedItem and IsTDExplosive(self.EquippedItem) and (self.EquippedItem:IsActivated() or self.controller:IsState(Controller.PRIMARY_ACTION)) then
		self.toThrowFoley = true;
	elseif self.toThrowFoley then
		self.toThrowFoley = false;
		BrowncoatBossFunctions.createVoiceSoundEffect(self, self.voiceSounds.Throw, 2, false);
		self.throwFoleySound:Play(self.Pos);
	end
	
	-- Squad leading VO
	-- this is imperfect: it'll only play when we go straight from being in a squad to being player controlled (i.e. we are now the leader)
	-- there is no way to see if we're leading a squad, so that close approximation is the best we can get.
	
	if self.AIMode == 11 then
		self.toPlaySquadLeadVoiceline = true;
	elseif self.toPlaySquadLeadVoiceline == true and self:IsPlayerControlled() and self.squadLeadVoiceLineTimer:IsPastSimMS(self.squadLeadVoiceLineDelay) then
		self.toPlaySquadLeadVoiceline = false;
		BrowncoatBossFunctions.createVoiceSoundEffect(self, self.voiceSounds.SquadLead, 3, false);
		self.squadLeadVoiceLineTimer:Reset();
	else
		self.toPlaySquadLeadVoiceline = false;
	end
end