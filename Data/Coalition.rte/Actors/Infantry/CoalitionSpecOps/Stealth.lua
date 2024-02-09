--[[
#### Stealth Script for the Spec Ops
#### The Spec Ops uses active camo to become virtually invisible to its enemies
#### If shot he will be revealed. He also gets revealed when reloading
#### He can operate guns while invisible
--]]


function Create(self)

	self.mapwrapx = SceneMan.SceneWrapsX;
	self.activeCamo = false;
	self.unstealth = false
	self.change = false
	
	self.CloakTimer = Timer();
	self.CloakDelayShort = 300;
	self.CloakDelayLong = 1000;
	self.CloakDelay = self.CloakDelayLong
	
	self.WTime = Timer();
	
	self.pulseTimer = Timer();
	self.pulseDelay = 2000;
	self.pulseDuration = 200
	
	self.jet = nil;

	for attch in self.Attachables do
		if attch.PresetName == "Jetpack Heavy" then
			self.jet = attch
		end
	end
	
	self.eyeYTable = {-1, 3}
	self.eyeXTable = {2,-3}
	
	self.shotThreshold = 5
	self.shotsFired = 0
	self.shotTimer = Timer();
	self.shotDelay = 600;

end


function Update(self)

	--IF THE UNIT IS NOT DEAD...
	if not(self:IsDead()) then

		--IF ACTIVE CAMO IS ON, DO THESE THINGS
		if self.activeCamo == true then
			
			------------ Effects -------------

			--CREATE RED EYE EFFECT
			
			if self.Head then
				for i = 1, #self.eyeYTable do
					for j = 1, #self.eyeXTable do
						local X = self.eyeXTable[j]
						local Y = self.eyeYTable[i]
						local redeyeOffset = Vector(X*self.FlipFactor, Y):RadRotate(self.Head.RotAngle);
						local redeye = CreateMOPixel("Coalition SpecOps Yellow Eye");
						redeye.Pos = self.Head.Pos + redeyeOffset
						MovableMan:AddParticle(redeye);
					end
				end

				
				if self.pulseTimer:IsPastSimMS(self.pulseDelay) then
					--CREATE RED EYE EFFECT
					for i = 1, #self.eyeYTable do
						for j = 1, #self.eyeXTable do
							local X = self.eyeXTable[j]
							local Y = self.eyeYTable[i]
							local redeyeOffset = Vector(X*self.FlipFactor, Y):RadRotate(self.Head.RotAngle);
							local redeye = CreateMOPixel("Coalition SpecOps Yellow Eye");
							redeye.Pos = self.Head.Pos + redeyeOffset
							MovableMan:AddParticle(redeye);
						end
					end
						
					if self.pulseTimer:IsPastSimMS(self.pulseDelay+self.pulseDuration) then
						self.pulseTimer:Reset();
					end
				end
			end
		else
			self.pulseTimer:Reset();
		end

		--------- Un-stealth checks ----------

		if self.WTime:IsPastSimMS(100) then
			self.WTime:Reset();
			--DETECTOR CHECK
			for mo in MovableMan:GetMOsInRadius(self.Pos, 200, self.Team) do
			
				--NEARVY ACTOR
				if IsActor(mo:GetRootParent()) then
					local actor = ToActor(mo:GetRootParent());

					local parvector = SceneMan:ShortestDistance(self.Pos,actor.Pos,self.mapwrapx);
					local percep = actor.Perceptiveness
					local shieldradius = 100
					
					if parvector:MagnitudeIsLessThan(shieldradius * percep) then
						self.CloakTimer:Reset();
						self.change = self.activeCamo;
						self.CloakDelay = self.CloakDelayShort;
					end
				end
				
				--GETTING SHOT
				if mo.HitsMOs == true and mo.Vel.Magnitude > 5 and (( (IsMOPixel(mo) or IsMOSParticle(mo))) 
				or ((IsMOSRotating(mo) or IsAEmitter(mo)) and ToMOSRotating(mo).GibImpulseLimit < 150 and not string.find(mo.PresetName, "Gib")))
				and (mo.Team ~= self.Team or (mo.Team == self.Team and mo.IgnoresTeamHits == true)) then
					
					local check = SceneMan:ShortestDistance(self.Pos,mo.Pos,self.mapwrapx);
					local checkFuture = SceneMan:ShortestDistance(self.Pos,mo.Pos+mo.Vel,self.mapwrapx);
					if check:MagnitudeIsLessThan(self.Radius + 35) or checkFuture:MagnitudeIsLessThan(self.Radius + 35)then
						self.CloakTimer:Reset();
						self.change = self.activeCamo;
						self.CloakDelay = self.CloakDelayLong;						
					end
				end				
				
			end
			--END DETECTOR CHECK
		end
		
		local controller = self:GetController();

		if self.Status == Actor.UNSTABLE or self.Vel.Magnitude > 10 then
			self.CloakTimer:Reset();
			self.change = self.activeCamo;
			self.CloakDelay = self.CloakDelayLong;
		end
		
		if controller:IsState(Controller.WEAPON_PICKUP) or controller:IsState(Controller.WEAPON_CHANGE_NEXT) or controller:IsState(Controller.WEAPON_CHANGE_PREV) then
			self.CloakTimer:Reset();
			self.change = self.activeCamo;
			self.CloakDelay = self.CloakDelayShort;
		end
		
		--SEE IF UNIT IS FIRING

		if self.shotTimer:IsPastSimMS(self.shotDelay) then
			self.shotTimer:Reset();
			self.shotsFired = self.shotsFired > 0 and self.shotsFired - 1 or 0
		end
		
		if self.EquippedItem then
			local item = self.EquippedItem
			if IsHDFirearm(item) then
				local gun = ToHDFirearm(item)
				
				if gun.FiredFrame then
					self.shotsFired = self.shotsFired + 1
					self.shotTimer:Reset();
--[[					
					if self.shotsFired == self.shotThreshold then
						self.CloakTimer:Reset();
						self.change = self.activeCamo;
						self.CloakDelay = self.CloakDelayLong;
						self.shotsFired = self.shotThreshold - 1
					end
--]]
				end
				
				if gun:IsReloading() then
					self.CloakTimer:Reset();
					self.change = self.activeCamo;
					self.CloakDelay = self.CloakDelayLong;
				end
			elseif IsTDExplosive(item) or IsThrownDevice(item) then
				--TO-DO: unstealth only when releasing grenade
				if self:GetController():IsState(Controller.WEAPON_FIRE) then
					self.CloakTimer:Reset();
					self.change = self.activeCamo;
					self.CloakDelay = self.CloakDelayShort;
				end
			end
		end
		
		if self.EquippedBGItem then
			local item = self.EquippedBGItem
			if IsHDFirearm(item) then
				local gun = ToHDFirearm(item)
				if gun:IsReloading() then
					self.CloakTimer:Reset();
					self.change = self.activeCamo;
					self.CloakDelay = self.CloakDelayLong;
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
					if object:IsDevice() then
						if object:HasScript("Coalition.rte/Actors/Infantry/CoalitionSpecOps/StealthItem.lua") == false then
							object:AddScript("Coalition.rte/Actors/Infantry/CoalitionSpecOps/StealthItem.lua");
						else
							object:EnableScript("Coalition.rte/Actors/Infantry/CoalitionSpecOps/StealthItem.lua");
						end
					else

						if self.activeCamo == true then
							object.Scale = 1;
						else
							object.Scale = 0;

							--CREATE RED EYE EFFECT
							if self.Head then
								for i = 1, #self.eyeYTable do
									for j = 1, #self.eyeXTable do
										local X = self.eyeXTable[j]
										local Y = self.eyeYTable[i]
										local redeyeOffset = Vector(X*self.FlipFactor, Y):RadRotate(self.Head.RotAngle);
										local redeye = CreateMOPixel("Coalition SpecOps Yellow Eye");
										redeye.Pos = self.Head.Pos + redeyeOffset
										MovableMan:AddParticle(redeye);
									end
								end
							end
						end
					end
				end
			end
			
			local attCount = 0

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
			
			if self.activeCamo == true then
				self.jet.Scale = 1
			else
				self.jet.Scale = 0;
			end

			if self.activeCamo == true then
				self.activeCamo = false;
				self.HUDVisible = true;
			else
				self.activeCamo = true;
				self.HUDVisible = false;
				self.shotsFired = 0
			end
			
			local effect = CreateMOSRotating("SpecOps Camo Effect", "Coalition.rte");
			effect.Pos = self.Pos;
			effect.Vel = Vector(0,-5)
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