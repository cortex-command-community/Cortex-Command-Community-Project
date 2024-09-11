function Create(self)
	self.captureSound = CreateSoundContainer("Dock Capture", "Base.rte");
	self.releaseSound = CreateSoundContainer("Dock Release", "Base.rte");

	self.saveLoadHandler = require("Activities/Utility/SaveLoadHandler");
	self.saveLoadHandler:Initialize(self);

	self.updateTimer = Timer();
	self.healTimer = Timer();
	self.HoldTimer = Timer();
	self.ReleaseTimer = Timer();
	
	self.HoldTime = 5000;
	self.ReleaseTime = 800;
	
	self.detectionRange = 55; -- Default 40
	
	self.visualDockDistance = 85; -- in pixels from the SpriteOffset, horizontally
	
	self.HasDockedCraft = false;
	self.confirmCapture = true;
	
	if self:StringValueExists("savedDockedCraft") then
		-- without also saving our timers, we will hold these guys for the full duration.
		-- but that's fine. who's gonna notice?
	
		self.craft = self.saveLoadHandler:LoadLocallySavedMO(self, "savedDockedCraft");
		self.HasDockedCraft = true;
	end
end

function Update(self)
	if UInputMan:KeyPressed(Key.U) then
		self:ReloadScripts();
	end

	--Settle into terrain instead of gibbing
	if self.WoundCount > (self.GibWoundLimit * 0.9) then
		self.ToSettle = true;
	end

	if self.craft and MovableMan:ValidMO(self.craft) then
		--This block runs before HoldTimer it's passed
		if not self.HoldTimer:IsPastSimMS(self.HoldTime) then
			if not self.HasDockedCraft then
				self.craft.AIMode = Actor.AIMODE_STAY;
				self.HasDockedCraft = true;
			end
			if self.craft.Status < Actor.DYING then
				self.craft.Status = Actor.UNSTABLE;	--Deactivated
			end
			self.ReleaseTimer:Reset(); -- During the time the HoldTimer isn't reached, prevent ReleaseTimer from going
		else
			if self.craft.Status < Actor.DYING then
				self.craft.Status = 0;
			end
		end

		if self.HasDockedCraft then
			if self.confirmCapture and self.HoldTimer:IsPastSimMS(650) then
				self.confirmCapture = false;
				
				self.craft:SetNumberValue("Docked", 1);

				for i = 1, 2 do				
					local direction = 1;
				
					if i == 2 then direction = -1 end;
					
					for i = 1, 8 do
						local particle = CreateMOSParticle("Small Smoke Ball 1", "Base.rte");
						particle.GlobalAccScalar = 0.005
						particle.Lifetime = math.random(800, 2500);
						particle.Vel = self.Vel + Vector(math.random(-20, 20)/100, -math.random(-40, -30)/100);
						particle.Pos = self.Pos + Vector(self.visualDockDistance * direction + math.random(-4, 4), math.random(-3, 3));
						MovableMan:AddParticle(particle);
					end
					
					for i = 1, 6 do
						local particle = CreateMOSParticle("Small Smoke Ball 1", "Base.rte");
						particle.GlobalAccScalar = 0.005
						particle.Lifetime = math.random(800, 2500);
						particle.Vel = self.Vel + Vector(math.random(-20, 20)/100, -math.random(-100, -30)/100);
						particle.Pos = self.Pos + Vector(self.visualDockDistance * direction + math.random(-4, 4), math.random(-3, 3));
						MovableMan:AddParticle(particle);
					end					
				end						
			end
		
			--Disable collisions with the ship
			self.craft:SetWhichMOToNotHit(self, 100);
			--Pin the ship and pull it nicely into the docking unit.
			local dist = SceneMan:ShortestDistance(self.craft.Pos, self.Pos + Vector(0, -self.craft.Radius * 0.5), SceneMan.SceneWrapsX);
			self.craft.Vel = self.craft.Vel * 0.9 + dist/(3 + self.craft.Vel.Magnitude);
			self.craft.AngularVel = self.craft.AngularVel * 0.9 - self.craft.RotAngle * 3;

			--Heal the craft
			if self.healTimer:IsPastSimMS(self.craft.Mass) then
				self.healTimer:Reset();
				if self.craft.WoundCount > 0 then
					self.craft:RemoveWounds(1);
				elseif self.craft.Health < self.craft.MaxHealth then
					self.craft.Health = math.min(self.craft.Health + 1, self.craft.MaxHealth);
				end
			end
		end

		if self.ReleaseTimer:IsPastSimMS(self.ReleaseTime) then -- Once ReleaseTimer stops resetting it can finally run
			self.craft.AIMode = Actor.AIMODE_RETURN
			self.craft.DeliveryState = ACraft.LAUNCH;
			self.craft.AltitudeMoveState = ACraft.ASCEND;
			self.craft:RemoveNumberValue("Docked");
			self.craft = nil; --Forget about the craft thus starting all over again
			self.ReleaseTimer:Reset();
			self.releaseSound:Play(self.Pos);

			for i = 1, 2 do
			
				local direction = 1;
			
				if i == 2 then direction = -1 end;
				
				for i = 1, 8 do
					local particle = CreateMOSParticle("Small Smoke Ball 1", "Base.rte");
					particle.GlobalAccScalar = 0.005
					particle.Lifetime = math.random(800, 2500);
					particle.Vel = self.Vel + Vector(math.random(-20, 20)/100, -math.random(-40, -30)/100);
					particle.Pos = self.Pos + Vector(self.visualDockDistance * direction + math.random(-4, 4), math.random(-3, 3));
					MovableMan:AddParticle(particle);
				end
				
				for i = 1, 6 do
					local particle = CreateMOSParticle("Small Smoke Ball 1", "Base.rte");
					particle.GlobalAccScalar = 0.005
					particle.Lifetime = math.random(800, 2500);
					particle.Vel = self.Vel + Vector(math.random(-20, 20)/100, -math.random(-100, -30)/100);
					particle.Pos = self.Pos + Vector(self.visualDockDistance * direction + math.random(-4, 4), math.random(-3, 3));
					MovableMan:AddParticle(particle);
				end	
			end
		end
	elseif self.ReleaseTimer:IsPastSimMS(self.ReleaseTime * 4) and self.updateTimer:IsPastSimMS(200) then
		self.craft = nil;
		for mo in MovableMan:GetMOsInRadius(self.Pos, self.detectionRange, -1, true) do
			--See if a live rocket is within 45 pixel range of the docking unit
			if mo.ClassName == "ACDropShip" and not ToActor(mo):IsDead() then
				self.craft = ToACDropShip(mo);
				self.confirmCapture = true;
				self.captureSound:Play(self.Pos);
			end
		end

		self.HoldTimer:Reset();
		self.updateTimer:Reset();
	end
end

function OnSave(self)
	if self.craft then
		self.saveLoadHandler:SaveMOLocally(self, "savedDockedCraft", self.craft);
	end
end