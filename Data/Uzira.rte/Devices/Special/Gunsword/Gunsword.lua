function Create(self)
	self.rotNum = 0;
	self.stance = Vector(6, 8);
	self.sharpStance = Vector(11, 2);
	self.length = self.Radius/rte.PxTravelledPerFrame;
	self.momentum = 0;
	self.charge = 0;
	self.throwSpeed = 0;
	self.swingSound = CreateSoundContainer("Uzira.rte/Swing");
	
	self.particle = CreateMOPixel("Base.rte/Smack Particle");
	self.particle.IgnoresTeamHits = true;
	self.particle:SetWhichMOToNotHit(self, -1);
	self.particle.Mass = 2;
	self.particle.Sharpness = (self.Sharpness/self.length/self.particle.Mass);
end

function Update(self)
	local sinAngle = math.sin(self.rotNum);
	local cosAngle = math.cos(self.rotNum);
	local parent = self:GetRootParent();
	local aimNum = 0;
	if IsAHuman(parent) then
		parent = ToAHuman(parent);
		aimNum = parent.SharpAimProgress * 2;
		local activated = self:IsActivated();
		local speed = 0.8 + (0.3 - cosAngle) * self.charge;
		self.throwSpeed = 0;
		if not self.Supported then
			speed = speed * 0.7;
		end
		
		if aimNum > 1 then
			self.HUDVisible = true;
			if activated then
				self.justFired = true;
			end
		else
			self.HUDVisible = false;
			if (self.rotNum > 0 or activated) and not self.justFired then
				parent:GetController():SetState(Controller.AIM_SHARP, false);
				if not parent:IsPlayerControlled() and sinAngle > 0.3 then
					parent:GetController():SetState(Controller.WEAPON_FIRE, false);
				end

				parent.AngularVel = parent.AngularVel + cosAngle * 0.3 * speed * self.FlipFactor;
				self.particle.Team = parent.Team;
				if cosAngle < 0 then	--Swing
					local swingProgress = (-cosAngle - sinAngle) * 0.7071;	--Max amplitude, i.e. sin or cos pi/4
					if swingProgress > 0 then
						self.particle.Pos = self.Pos;
						local swingSpeed = speed^(2 + swingProgress);
						for i = 1, math.floor(swingSpeed + math.sqrt(self.momentum)) do
							local part = self.particle:Clone();
							part.Mass = part.Mass * swingProgress;
							part.Vel = self.Vel + Vector(self.length * self.FlipFactor, 0):RadRotate(self.RotAngle - math.sqrt(i) * 0.5 * self.FlipFactor) * math.sqrt(swingProgress);
							MovableMan:AddParticle(part);
						end

						parent.Vel = parent.Vel + Vector(swingSpeed/(5 + self.momentum), 0):RadRotate(parent:GetAimAngle(true));
						if not self.swingSound:IsBeingPlayed() then
							self.swingSound.Pos = self.Pos;
							self.swingSound:Play();
							self:RemoveWounds(1);
						end

						self.throwSpeed = (speed * swingProgress) * 4;
					end
				elseif activated then
					speed = speed * cosAngle * 0.6;
					self.charge = math.abs(sinAngle);
				else
					self.swingSound.Pitch = math.sqrt(speed);
				end

				self:Deactivate();
				
				local twoPI = math.pi * 2;
				if self.rotNum < twoPI then
					self.rotNum = math.min(self.rotNum + speed * 0.2, twoPI);
				else
					self.rotNum = 0;
				end

				if self.Magazine then
					self.Magazine.Scale = 1;
					self.Scale = 0;
				else
					self.Scale = 1;
				end
			else
				if self.Magazine then
					self.Magazine.Scale = 0;
				end

				self.Scale = 1;
			end
		end

		if self:DoneReloading() or not parent:GetController():IsState(Controller.AIM_SHARP) then
			self.justFired = false;
		end
		
		local stance = Vector(self.stance.X * (3 - cosAngle - math.abs(sinAngle)), self.stance.Y):RadRotate(sinAngle * 1.5);
		self.StanceOffset = stance;
		self.SharpStanceOffset = self.sharpStance * math.max(aimNum, 0.7);
		self.InheritedRotAngleOffset = (math.sin(self.rotNum - 0.3) * (1.4 - cosAngle * 0.6) + 1.2 + math.abs(math.sin(parent.RotAngle * parent.FlipFactor) * 0.3)) * math.max(1 - aimNum, 0);
	else
		if self.Magazine then
			self.Magazine.Scale = 0;
		end

		self.Scale = 1;
		self.rotNum = 0;
		if self.throwSpeed > 0 then
			self.particle.Pos = self.Pos;
			local orientation = math.cos(self.RotAngle - self.Vel.AbsRadAngle) * self.FlipFactor;
			for i = 1, math.floor(math.sqrt(self.momentum) * 0.3) do
				local part = self.particle:Clone();
				part.Sharpness = part.Sharpness * (orientation + 1) * 0.5;
				part.Vel = self.Vel + Vector(self.length * self.FlipFactor, 0):RadRotate(self.RotAngle - math.sqrt(i - 1) * RangeRand(-0.3, 0.3)) * orientation;
				MovableMan:AddParticle(part);
			end
		end
	end

	self.momentum = (self.momentum + self.Vel.Magnitude * math.max(-self.AngularVel * self.FlipFactor, 1)) * 0.5;
end

function OnDetach(self, exParent)
	if self.throwSpeed > 0 and IsArm(exParent) then
		local throwAngle = exParent.RotAngle;
		local actor = exParent:GetRootParent();
		if actor and IsAHuman(actor) then
			self.user = ToAHuman(actor);
			throwAngle = self.user:GetAimAngle(false);
			self.Team = self.user.Team;
			self.IgnoresTeamHits = true;
		end

		self.Vel = self.Vel + Vector(self.throwSpeed * math.sqrt(math.abs(ToArm(exParent).ThrowStrength)) * exParent.FlipFactor, 0):RadRotate(throwAngle * self.FlipFactor);
		self.AngularVel = -self.Vel.Magnitude * self.FlipFactor;
		self.throwSpeed = self.Vel.Magnitude;
	end
end

function OnCollideWithMO(self, mo, rootMO)
	if self.user then
		if self.throwSpeed > 1 and IsActor(self.user) and self.momentum > 10 then
			self.Vel = (self.Vel + SceneMan:ShortestDistance(self.Pos, self.user.Pos, SceneMan.SceneWrapsX):SetMagnitude(self.throwSpeed)) * 0.3;
			self.AngularVel = self.AngularVel * -0.6;
			
			self.particle.Pos = self.Pos;
			local part = self.particle:Clone();
			local dist = SceneMan:ShortestDistance(self.Pos, mo.Pos, SceneMan.SceneWrapsX);
			part.Vel = Vector(dist.X, dist.Y) * 3;
			part.Mass = part.Mass/math.max(part.Vel.Magnitude, 1);
			part.Sharpness = part.Sharpness * self.length;
			MovableMan:AddParticle(part);
			mo:AddForce(self.PrevVel * (self.Mass + self.momentum), Vector());
		else
			self.Vel = (self.Vel + SceneMan:ShortestDistance(self.Pos, self.user.Pos, SceneMan.SceneWrapsX):SetMagnitude(self.throwSpeed)) * 0.3;
			self.user = nil;
		end

		self.throwSpeed = 1;
		self:SetWhichMOToNotHit(rootMO, -1);
	end
end