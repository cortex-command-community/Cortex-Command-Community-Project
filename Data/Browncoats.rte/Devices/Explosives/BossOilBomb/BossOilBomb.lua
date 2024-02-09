function OnDetach(self)
	if not self.activated then
		self.ToDelete = true;
	end
end

function Create(self)
	self.thrownSound = CreateSoundContainer("Browncoat Boss Oil Bomb Thrown", "Browncoats.rte");
	
	self.fuzeDelay = 1500;
	
	self.throwForcedDelay = 1000;
	
	self.HUDVisible = false;
end

function Update(self)
	self.thrownSound.Pos = self.Pos;
	
	if self:IsAttached() then
		self.controller = ToActor(self:GetRootParent()):GetController();
	else
		self.controller = nil;
	end

	if self.fuze then
		if self.fuze:IsPastSimMS(self.fuzeDelay) then
			self:GibThis();
			local igniter = CreateMOSRotating("Browncoat Boss Oil Bomb Igniter", "Browncoats.rte");
			igniter.Pos = self.Pos
			igniter.HFlipped = self.HFlipped;
			igniter.Vel = self.Vel
			igniter.AngularVel = self.AngularVel
			igniter.RotAngle = self.RotAngle
			igniter.Team = self.Team;
			MovableMan:AddParticle(igniter);
		end
	elseif self.activated then
		if not self:IsAttached() then
			self.HUDVisible = false;
		
			self.thrownSound:Play(self.Pos);
		
			self.Frame = 1;
			self.fuze = Timer();
			
			self.AngularVel = self.AngularVel + -5*self.FlipFactor;

			for i = 1, 2 do
				local dupe = CreateMOSRotating("Browncoat Boss Oil Bomb Secondary", "Browncoats.rte");
				local offset = i == 1 and 10 or -10;
				dupe.Pos = self.Pos + Vector(0, offset);
				dupe.HFlipped = self.HFlipped;
				dupe.Vel = self.Vel + Vector(0, offset*0.15*self.FlipFactor):RadRotate(self.Vel.AbsRadAngle);
				dupe.AngularVel = self.AngularVel*RangeRand(0.8, 1.2);
				dupe.RotAngle = self.RotAngle*RangeRand(0.4, 1.6);
				dupe.Lifetime = dupe.Lifetime*RangeRand(0.95, 1.05) + 100;
				dupe.Team = self.Team;
				MovableMan:AddParticle(dupe);
			end
		end
		
		if self.controller and not self.throwDelayTimer:IsPastSimMS(self.throwForcedDelay) then
			self.controller:SetState(Controller.PRIMARY_ACTION, true);
			self.controller:SetState(Controller.WEAPON_FIRE, true);
			
			self.controller:SetState(Controller.WEAPON_CHANGE_NEXT, false);
			self.controller:SetState(Controller.WEAPON_CHANGE_PREV, false);
			self.controller:SetState(Controller.WEAPON_DROP, false);
		end
		
	elseif self:IsAttached() and (self.controller:IsState(Controller.PRIMARY_ACTION) or self:IsActivated()) then
		self.activated = true;
		self.throwDelayTimer = Timer();
	end
end