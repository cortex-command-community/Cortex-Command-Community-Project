require("AI/NativeHumanAI");

function Create(self)

	self.activity = ToGameActivity(ActivityMan:GetActivity())

	-- AI Overrides
	
	self:SetNumberValue("AIDisableProne", 1);
	
	self:SetNumberValue("AIAimSpeed", 0.04);
	self:SetNumberValue("AIAimSkill", 0.04);
	self:SetNumberValue("AISkill", 100);

	self.AI = NativeHumanAI:Create(self);
	
	local mainWeapon = CreateHDFirearm("MG-85 Manbreaker", "Browncoats.rte");
	mainWeapon:SetNumberValue("Boss Mode", 1);
	mainWeapon.HitsMOs = false;
	mainWeapon.GetsHitByMOs = false;
	mainWeapon.MissionCritical = true;
	self:AddInventoryItem(mainWeapon);
	
	self.HUDVisible = false;
	
	self.PainThreshold = 4;
	
	-- we do this instead of attachable iterating to avoid making our armor invulnerable
	
	self.MissionCritical = true;
	
	self.GibImpulseLimit = 99999999;
	self.ImpulseDamageThreshold = 99999999;
	self.GibWoundLimit = 99999999;

	self.Head.MissionCritical = true;
	self.Head.GibImpulseLimit = 99999999;
	self.Head.ImpulseDamageThreshold = 99999999;
	self.Head.GibWoundLimit = 99999999;	
	
	self.FGArm.MissionCritical = true;
	self.FGArm.GibImpulseLimit = 99999999;
	self.FGArm.ImpulseDamageThreshold = 99999999;
	self.FGArm.GibWoundLimit = 99999999;
	
	self.BGArm.MissionCritical = true;
	self.BGArm.GibImpulseLimit = 99999999;
	self.BGArm.ImpulseDamageThreshold = 99999999;
	self.BGArm.GibWoundLimit = 99999999;

	self.FGLeg.MissionCritical = true;
	self.FGLeg.GibImpulseLimit = 99999999;
	self.FGLeg.ImpulseDamageThreshold = 99999999;
	self.FGLeg.GibWoundLimit = 99999999;
	
	self.BGLeg.MissionCritical = true;
	self.BGLeg.GibImpulseLimit = 99999999;
	self.BGLeg.ImpulseDamageThreshold = 99999999;
	self.BGLeg.GibWoundLimit = 99999999;
	
	self.jumpPackSound = CreateSoundContainer("Browncoat Boss Jump Pack", "Browncoats.rte");
	
	self.abilityShockwaveWhooshSound = CreateSoundContainer("Browncoat Boss Ability Shockwave Whoosh", "Browncoats.rte");	
	self.abilityShockwaveLandSound = CreateSoundContainer("Browncoat Boss Ability Shockwave Land", "Browncoats.rte");	
	
	self.abilityShockwaveTimer = Timer();
	self.abilityShockwaveJumpPackDelay = 300;
	
	self.abilityShockwaveScreenShakeAmount = 30;
	self.abilityShockwaveRange = 1500;
	self.abilityShockwaveStrength = 700;
	
	self.quickThrowTimer = Timer();
	self.quickThrowDelay = 10000;
	self.quickThrowExplosive = CreateTDExplosive("Browncoat Boss Oil Bomb", "Browncoats.rte");	
	
	self.deathScripted = false;
	self.deathScriptedTimer = Timer();
	
	self.deathScriptedMidBurnDelay = 1700;
	self.deathScriptedExplodeDelay = 3340;
	
	self.deathScriptedStartSound = CreateSoundContainer("Browncoat Boss DeathScriptedStart", "Browncoats.rte");
	self.deathScriptedMidBurnSound = CreateSoundContainer("Browncoat Boss DeathScriptedMidBurn", "Browncoats.rte");
	self.deathScriptedExplodeSound = CreateSoundContainer("Browncoat Boss DeathScriptedExplode", "Browncoats.rte");	

	
end

function Update(self)
	self.abilityShockwaveWhooshSound.Pos = self.Pos;
	self.jumpPackSound.Pos = self.Pos;
	
	local debugHealthTrigger = UInputMan:KeyPressed(Key.N);
	
	if debugHealthTrigger then
		self.Health = self.Health - 20;
	end

	if not self:IsDead() then
		-- Replenish our oil throw ability to always have it ready
	
		if not self.deathScripted == true then
			if not self:HasObjectInGroup("Bombs") then 
				local explosive = self.quickThrowExplosive:Clone();
				self:AddInventoryItem(explosive);
			end
		end

		-- The boss LMG already has its own OnAttach delay and animation, but might as well do it here too for a post-quickthrow pause
		if not self.quickThrowTimer:IsPastSimMS(2000) then
			self.controller:SetState(Controller.PRIMARY_ACTION, false);
			if self.EquippedItem then
				self.EquippedItem:Deactivate();
			end
		end
		
		-- Boss health bar
		
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do	
			if self.activity:PlayerActive(player) and self.activity:PlayerHuman(player) then
				local pos = CameraMan:GetOffset(player);
				pos.X = pos.X + FrameMan.PlayerScreenWidth * 0.5;
				--print(pos)
				local yOffset = FrameMan.PlayerScreenHeight * 0.87;
				local xOffset = Vector(FrameMan.PlayerScreenWidth * 0.33, 0);
				pos.Y = pos.Y + yOffset
			
				local colors = {244, 46, 47, 48, 86, 87, 118, 135, 149, 162, 147}
				local fac = math.max(math.min(self.Health / self.MaxHealth, 1), 0)
				local color = colors[math.floor(fac * (#colors - 1) + 1.5)]
				
				local textPos = Vector(pos.X, pos.Y - 20);
				PrimitiveMan:DrawTextPrimitive(textPos, "SERGEV, OIL BARON", false, 1)
				
				-- Bar Background
				PrimitiveMan:DrawLinePrimitive(pos - xOffset, pos + xOffset, 26, 10);
				-- Bar Foreground
				PrimitiveMan:DrawLinePrimitive(pos - xOffset, pos - xOffset + Vector(xOffset.X * 2 * fac, 0), color, 10);
			end
		end		
	end
	
	-- Death sequence
	
	if self.deathScripted then
		self.Health = 1;
		self.Status = 0;
	
		self.deathScriptedStartSound.Pos = self.Pos;
		self.deathScriptedMidBurnSound.Pos = self.Pos;
		self.deathScriptedExplodeSound.Pos = self.Pos;
		
		-- Note: set to ignore play, so we don't have to check if it's already been played
		self.deathScriptedStartSound:Play(self.Pos);
		
		if self.deathScriptedTimer:IsPastSimMS(self.deathScriptedExplodeDelay) then
			self.deathScriptedExplodeSound:Play(self.Pos);
			self.MissionCritical = false;
			for att in self.Attachables do
				att.MissionCritical = false;
			end
			
			self:GibThis();
			
			CameraMan:AddScreenShake(25, self.Pos);
		elseif self.deathScriptedMidBurnDone ~= true and self.deathScriptedTimer:IsPastSimMS(self.deathScriptedMidBurnDelay) then
			self.deathScriptedMidBurnDone = true;
			-- played via BurstSound
			--self.deathScriptedMidBurnSound:Play(self.Pos);
			
			local offset = Vector(0, -3)
			
			local emitterB = CreateAEmitter("Browncoat Boss Scripted Death Burn")
			emitterB.InheritedRotAngleOffset = math.pi/2;
			self.Jetpack:AddAttachable(emitterB);
			
			ToAttachable(emitterB).ParentOffset = offset
		end
		
		if self.deathScriptedMidBurnDone then
			CameraMan:AddScreenShake(0.8, self.Pos);
		end
	end
		
	
	-- DEBUG SHOCKWAVE ABILITY
	
	local debugTrigger = UInputMan:KeyPressed(Key.O);
	
	if debugTrigger then
		self.abilityShockwaveTrigger = true;
		self.abilityShockwaveJumpPacked = false;
	end
	
	if self.abilityShockwaveOngoing then
		if not self.abilityShockwaveJumpPacked and self.abilityShockwaveTimer:IsPastSimMS(self.abilityShockwaveJumpPackDelay) then
			self.abilityShockwaveJumpPacked = true;
			
			-- Old actual jetpack method
			--self.Jetpack.NegativeThrottleMultiplier = self.jumpPackDefaultNegativeMult * 1.3;
			--self.Jetpack.PositiveThrottleMultiplier = self.jumpPackDefaultPositiveMult * 1.3;
			
			self.controller:SetState(Controller.BODY_JUMPSTART, true)
			self.controller:SetState(Controller.BODY_JUMP, true)
			
			self.jumpPackSound:Play(self.Pos);
			
			self.Vel = self.Vel + Vector(2 * self.FlipFactor, -10);
			
			local offset = Vector(0, -3)
			
			local emitterA = CreateAEmitter("Browncoat Boss JumpPack Smoke Trail Medium")
			emitterA.InheritedRotAngleOffset = math.pi/2;
			self.Jetpack:AddAttachable(emitterA);
			
			ToAttachable(emitterA).ParentOffset = offset
			
			local emitterB = CreateAEmitter("Browncoat Boss JumpPack Smoke Trail Heavy")
			emitterB.InheritedRotAngleOffset = math.pi/2;
			self.Jetpack:AddAttachable(emitterB);
			
			ToAttachable(emitterB).ParentOffset = offset
			
			if not self.abilityShockwaveWhooshSound:IsBeingPlayed() then
				self.abilityShockwaveWhooshSound:Play(self.Pos);
			end
		end
	end
end

function UpdateAI(self)
	-- Quick throw AI trigger on a timer

	if not self:IsPlayerControlled() then -- just in case
		if self.quickThrowTimer:IsPastSimMS(self.quickThrowDelay) then
			if not (self.EquippedItem and self.EquippedItem:IsReloading() or self.EquippedItem:NumberValueExists("Busy")) then
				if self.AI:CreateQuickthrowBehavior(self) then
					self.quickThrowTimer:Reset();
					BrowncoatBossFunctions.createVoiceSoundEffect(self, self.voiceSounds.OilThrowTaunt, 10, true);
				end
			end
		end
	end

	self.AI:Update(self);
end

function Destroy(self)
	self.AI:Destroy(self);
	self.activity:SendMessage("Refinery_S10FinalBossDead");
end