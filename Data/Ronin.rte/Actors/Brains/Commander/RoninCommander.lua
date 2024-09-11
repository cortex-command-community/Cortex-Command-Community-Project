function Create(self)
	self.updateTimer = Timer();

	self.lungeTapDelay = 200;
	self.lungePower = 6;
	self.tapTimer = Timer();
	self.dir = 0;

	if not self:NumberValueExists("Identity") then
		self.face = math.random(0, self.FrameCount - 2);
		self.Frame = self.face;
		if self.Head then
			self.Head.Frame = self.face;
		end
		
		self:SetNumberValue("Identity", self.face);
		--Add a ponytail if we are Lara
		if self.face == 2 then
			self.DeathSound.Pitch = 1.2;
			self.PainSound.Pitch = 1.2;
			if self.Head then
				self.Head:AddAttachable(CreateAttachable("Ronin Brunette Ponytail"));
			end
		end
	else
		self.face = self:GetNumberValue("Identity");
		if self.Head then
			self.Head.Frame = self.face;
		end
	end

	self.stableVel = self:GetStableVelocityThreshold();

	function self.lunge(power)
		local flip = 0;
		if self.Status == Actor.STABLE then
			flip = self.FlipFactor;
			if self.controller:IsState(Controller.MOVE_RIGHT) then
				flip = 1;
			elseif self.controller:IsState(Controller.MOVE_LEFT) then
				flip = -1;
			end
			--Different factors that affect the lunge
			local angVel = math.abs(self.AngularVel * 0.1) + 1;
			local vel = (self.Vel.Magnitude + angVel)^2 * 0.0005 + 1;
			local mass = math.abs(self.Mass * 0.005) + 1;
			local aimAng = self:GetAimAngle(false);
			local vertical = math.abs(math.cos(aimAng))/vel;
			local strength = power * math.min(self.Health/self.MaxHealth, 1);

			local jumpVec =	Vector((power + strength/vel) * flip, -(power * 0.5 + (strength * 0.3)) * vertical):RadRotate(aimAng * self.FlipFactor);

			self.Vel = self.Vel + jumpVec/mass;
			self.AngularVel = self.AngularVel - (1/angVel * vertical) * flip * math.cos(self.RotAngle);
			self.Status = Actor.UNSTABLE;
			self.tapTimer:Reset();
		end

		return flip;
	end
end

function Update(self)
	self.controller = self:GetController();
	if self.updateTimer:IsPastSimMS(1000) then
		self.updateTimer:Reset();
		self.aggressive = self.Health < self.MaxHealth * 0.5;
	end

	if self.Head then
		self.Head.Frame = self.face;
		if (self.controller:IsState(Controller.WEAPON_FIRE) and not self:FirearmsAreReloading(false)) or self.aggressive or self.Health < self.PrevHealth - 1 then
			self.Head.Frame = self.face + (self.Head.FrameCount * 0.5);
		end
	end

	self.Frame = (self.face == 2 and self.Vel.Y > (SceneMan.GlobalAcc.Y * 0.15)) and 3 or self.face;
	local crouching = false;
	if self:IsPlayerControlled() and self.Status < Actor.INACTIVE and self.FGLeg and self.BGLeg then
		crouching = self.controller:IsState(Controller.BODY_CROUCH);
		if self.Status == Actor.UNSTABLE then
			if self.dir == self.FlipFactor then
				local motion = (self.Vel.Magnitude * 0.5 + math.abs(self.AngularVel));
				local stillness = 1/(1 + motion);
				if crouching then
					self.AngularVel = self.AngularVel * (1 - stillness) - (self.RotAngle - (self:GetAimAngle(false) - math.pi * 0.5) * self.FlipFactor) * 2 * stillness;
				end
			else
				self.dir = 0;
			end

			crouching = false;
		elseif crouching then
			if not self.crouchHeld then
				if not self.tapTimer:IsPastSimMS(self.lungeTapDelay) and SceneMan.Scene.GlobalAcc:MagnitudeIsGreaterThan(10) then
					self.dir = self.lunge(self.lungePower);
				end
				self.tapTimer:Reset();
				self.controller:SetState(Controller.BODY_CROUCH, false);
			end
		end

		self.crouchHeld = crouching;
	end

	self:SetStableVelocityThreshold(crouching and self.stableVel * (1 + math.cos(self.RotAngle)) or self.stableVel);
end