--[[
#### Stealth Script for the Chrome Ghost
#### The Chrome Ghost phases out of existance to become invisible
#### While invisible he can't be hurt. He can reload while invisible
#### He has to become visible to fire
--]]


function Create(self)

	self.mapwrapx = SceneMan.SceneWrapsX;
	self.activeCamo = false;
	self.change = false
	
	self.CloakTimer = Timer();
	self.CloakDelayGlobal = 600
	self.CloakDelay = self.CloakDelayGlobal
	
	self.WTime = Timer();
	
	self.pulseTimer = Timer();
	self.pulseDelay = 2000;
	self.pulseDuration = 200
	
	self.jet = nil;

	for attch in self.Attachables do
		if attch.PresetName == "Techion Jetpack" then
			self.jet = attch
		end
	end
	
	for attch in self.Head.Attachables do
		if attch.PresetName == "Techion Chrome Ghost Visor" then
			self.visor = attch
		end
	end
	
	self.normalPath = self:GetLimbPathSpeed(1)
	self.phasePath = self.normalPath*2

	self.normalPush = self.LimbPathPushForce
	self.phasePush = self.LimbPathPushForce + 10000
	
	self.normalWalkRot = self:GetRotAngleTarget(AHuman.WALK)
	self.phaseWalkRot = self.normalWalkRot -0.65
	
			
end


function Update(self)

	--IF THE UNIT IS NOT DEAD...
	if not(self:IsDead()) then

		--IF ACTIVE CAMO IS ON, DO THESE THINGS
		if self.activeCamo == true then
		
			------------ Effects -------------
			
			local redeyeOffset = Vector(3*self.FlipFactor, -3):RadRotate(self.RotAngle);
			local redeye = CreateMOPixel("Techion Chrome Ghost Blue Eye");
			redeye.Pos = self.Pos + redeyeOffset
			MovableMan:AddParticle(redeye);
							

			--CREATE RED EYE EFFECT
			if self.Head and self.visor:IsAttached() then
				local redeyeOffset = Vector(5*self.FlipFactor, -1):RadRotate(self.Head.RotAngle);
				local redeye = CreateMOPixel("Techion Chrome Ghost Blue Eye");
				redeye.Pos = self.Head.Pos + redeyeOffset
				MovableMan:AddParticle(redeye);

				if self.pulseTimer:IsPastSimMS(self.pulseDelay) then
					--CREATE RED EYE EFFECT
					local redeyeOffset = Vector(5*self.FlipFactor, -1):RadRotate(self.Head.RotAngle);
					local redeye = CreateMOPixel("Techion Chrome Ghost Blue Eye 2");
					redeye.Pos = self.Head.Pos + redeyeOffset
					MovableMan:AddParticle(redeye);
					
					if self.pulseTimer:IsPastSimMS(self.pulseDelay+self.pulseDuration) then
						self.pulseTimer:Reset();
					end
				end
			end
		else
			self.pulseTimer:Reset();
		end

		--------- Un-stealth checks ----------

		if self.WTime:IsPastSimMS(250) then
			self.WTime:Reset();
			--DETECTOR CHECK
			for mo in MovableMan:GetMOsInRadius(self.Pos, 150, self.Team) do
				if IsActor(mo:GetRootParent()) then
					local actor = ToActor(mo:GetRootParent());

					local parvector = SceneMan:ShortestDistance(self.Pos,actor.Pos,self.mapwrapx);
					local percep = actor.Perceptiveness
					local shieldradius = 65
					
					if parvector:MagnitudeIsLessThan(shieldradius * percep) then
						self.CloakTimer:Reset();
						self.change = self.activeCamo;
						self.CloakDelay = self.CloakDelayGlobal;
					end
				end
			end
			--END DETECTOR CHECK
		end			
		
		local controller = self:GetController();
		
		if controller:IsState(Controller.WEAPON_PICKUP) or controller:IsState(Controller.WEAPON_CHANGE_NEXT) or controller:IsState(Controller.WEAPON_CHANGE_PREV)
		or self.Status == Actor.UNSTABLE or self.Vel.Magnitude > 20 then
			self.CloakTimer:Reset();
			self.change = self.activeCamo;
			self.CloakDelay = self.CloakDelayGlobal;
		end

		--SEE IF UNIT IS FIRING
		
		if self.EquippedItem then
			local item = self.EquippedItem
			if IsHDFirearm(item) then
				local gun = ToHDFirearm(item)
				if gun.FiredFrame then
					self.CloakTimer:Reset();
					self.change = self.activeCamo;
					self.CloakDelay = self.CloakDelayGlobal;
				end
			elseif IsTDExplosive(item) then
				if self:GetController():IsState(Controller.WEAPON_FIRE) then
					self.CloakTimer:Reset();
					self.change = self.activeCamo;
					self.CloakDelay = self.CloakDelayGlobal;
				end
			end
		end
		
		if self.EquippedBGItem then
			local item = self.EquippedBGItem
			if IsHDFirearm(item) then
				local gun = ToHDFirearm(item)
				if gun.FiredFrame then
					self.CloakTimer:Reset();
					self.change = self.activeCamo;
					self.CloakDelay = self.CloakDelayGlobal;
				end
			end
		end
		
		-----

		if self.CloakTimer:IsPastSimMS(self.CloakDelay) and self.activeCamo == false then
			self.change = true
		end

		--INVISIBILITY SYSTEM: HANDLE ACTUAL INVISIBILITY CHANGE
		if self.change == true then
			for i = 0, MovableMan:GetMOIDCount() do
				if MovableMan:GetRootMOID(i) == self.ID then				
					local object = MovableMan:GetMOFromID(i);

					if IsHeldDevice(object) then
						if object:HasScript("Techion.rte/Actors/Infantry/ChromeGhost/StealthItem.lua") == false then
							object:AddScript("Techion.rte/Actors/Infantry/ChromeGhost/StealthItem.lua");
						else
							object:EnableScript("Techion.rte/Actors/Infantry/ChromeGhost/StealthItem.lua");
						end
					else

						if self.activeCamo == true then
							object.Scale = 1;
						else
							object.Scale = 0;

							--CREATE RED EYE EFFECT
							if self.Head and self.visor then
								local redeyeOffset = Vector(5*self.FlipFactor, -1):RadRotate(self.Head.RotAngle);
								local redeye = CreateMOPixel("Techion Chrome Ghost Blue Eye 2");
								redeye.Pos = self.Head.Pos + redeyeOffset
								MovableMan:AddParticle(redeye);
							end
						end
					end
				end
			end
			
			------------------------------------------
			
			for wound in self.Wounds do
				if self.activeCamo == true then
					wound.Scale = 1;
				else
					wound.Scale = 0;
				end
			end
			
			for attach in self.Attachables do
				for wound in attach.Wounds do
					if self.activeCamo == true then
						wound.Scale = 1;
					else
						wound.Scale = 0;
					end
				end
				for attach2 in attach.Attachables do
					for wound in attach2.Wounds do
						if self.activeCamo == true then
							wound.Scale = 1;
						else
							wound.Scale = 0;
						end
					end
				end
			end
			
			--------------------------------------------
			
			if self.activeCamo == true then
				self.jet.Scale = 1
			else
				self.jet.Scale = 0;
			end

			if self.activeCamo == true then
				self.activeCamo = false;
				self.HUDVisible = true;
				
				self:SetLimbPathSpeed(1, (self.normalPath))
				self:SetRotAngleTarget(AHuman.WALK, self.normalWalkRot)			
				self.LimbPathPushForce = self.normalPush		
			else
				self.activeCamo = true;
				self.HUDVisible = false;
				
				self:SetLimbPathSpeed(1, (self.phasePath))
				self:SetRotAngleTarget(AHuman.WALK, self.phaseWalkRot)			
				self.LimbPathPushForce = self.phasePush				
			end
			
			local effect = CreateMOSRotating("Chrome Ghost Cloak Effect", "Techion.rte");
			effect.Pos = self.Pos;
			effect.Team = self.Team
			effect.IgnoreTeamHits = true;
			MovableMan:AddParticle(effect);
			effect:GibThis();

			self.change = false;
		end

	else

		--...IF THE UNIT IS DEAD, PERFORM END LIFE FUNCTIONS
		for i = 0, MovableMan:GetMOIDCount() do
			if MovableMan:GetRootMOID(i) == self.ID then
				local object = MovableMan:GetMOFromID(i);
				object.Scale = 1;
			end
		end
		self.HUDVisible = true;
	end
end