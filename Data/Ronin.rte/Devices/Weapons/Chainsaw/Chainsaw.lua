function Create(self)
	self.rotFactor = 0;
	self.lastAngle = 0;
	self.origFireRate = self.RateOfFire;
	self.origParticleSpread = self.ParticleSpreadRange;

	self.fired = false;

	self.actTimer = Timer();
	self.activated = false;
	self.actDelay = 200;

	self.dismemberStrength = 250;
	self.length = ToMOSprite(self):GetSpriteWidth();

	self.startSound = CreateSoundContainer("Ronin Chainsaw Start", "Ronin.rte");
	self.stopSound = CreateSoundContainer("Ronin Chainsaw End", "Ronin.rte");
end

function Update(self)
	local turn = math.abs(self.AngularVel);
	local actor = MovableMan:GetMOFromID(self.RootID);
	if actor and IsAHuman(actor) then

		local parent = ToAHuman(actor);
		parent:GetController():SetState(Controller.AIM_SHARP, false);
		turn = math.abs(self.lastAngle - parent:GetAimAngle(false));

		self.InheritedRotAngleOffset = -(-0.8 + math.sin(self.rotFactor) * 0.4);

		self.Scale = 1;
		if self.Magazine then
			if self:IsActivated() then
				self.Scale = 0;
				self.activated = true;
				self.actTimer:Reset();
			end

			if self.activated then

				if self.rotFactor < 1 then
					self.rotFactor = self.rotFactor + 0.1;
				elseif self.rotFactor > 1 then
					self.rotFactor = 1;
				end
			else
				self.Scale = 1;

				if self.rotFactor > 0 then
					self.rotFactor = self.rotFactor - 0.1;
				elseif self.rotFactor < 0 then
					self.rotFactor = 0;
				end

				if self.Magazine.RoundCount < self.Magazine.Capacity then
					self.Magazine.RoundCount = self.Magazine.RoundCount + 1;
				end
			end
		end

		self.actDelay = 200;

		self.StanceOffset = Vector(10 + self.rotFactor * 3, 1):RadRotate(math.sin(self.rotFactor * 0.3) - 0.3);

		self.lastAngle = parent:GetAimAngle(true);
	else
		self.Scale = 1;
		if self.activated == true then
			self.actDelay = 2000;
		end

		self.lastAngle = self.RotAngle;
	end
	if self.Magazine then
		if self.activated == true then
			if self.Magazine.RoundCount ~= 0 then
				if self.actTimer:IsPastSimMS(self.actDelay) then
					self:Deactivate();
					self.activated = false;
				else
					self:Activate();
					self.AngularVel = self.AngularVel * 0.99;
				end
			end

			self.RateOfFire = self.origFireRate + 9 * (math.sqrt(turn * 100) + math.sqrt(self.Vel.Largest * 100));
			self.ParticleSpreadRange = self.origParticleSpread + 3 * (math.sqrt(turn * 100) + math.sqrt(self.Vel.Largest));
		end
		
		if self:IsActivated() and self.Magazine.RoundCount ~= 0 then
			self.Scale = 0;
			self.fired = true;
			--Dismemberment: detach limbs via MO detection
			local moCheck = SceneMan:CastMORay(self.Pos, Vector(self.length * 0.8 * self.FlipFactor, 0):RadRotate(self.RotAngle), self.ID, self.Team, rte.airID, true, 2);
			if moCheck ~= rte.NoMOID then
				local mo = MovableMan:GetMOFromID(moCheck);
				if mo and IsAttachable(mo) and ToAttachable(mo):IsAttached() and not (IsHeldDevice(mo) or IsThrownDevice(mo)) then
					mo = ToAttachable(mo);
					local jointPos = mo.Pos + Vector(mo.JointOffset.X * mo.FlipFactor, mo.JointOffset.Y):RadRotate(mo.RotAngle);
					if SceneMan:ShortestDistance(self.Pos, jointPos, SceneMan.SceneWrapsX):MagnitudeIsLessThan(3) and math.random(self.dismemberStrength) > mo.JointStrength then
						ToMOSRotating(mo:GetParent()):RemoveAttachable(mo.UniqueID, true, true);
					end
				end
			end
		elseif self.fired == true then
			self.Scale = 1;
			self.fired = false;

			self.stopSound:Play(self.Pos);

			if self.Magazine.RoundCount == 0 then
				self:Reload();
			end
		end
	else
		self.activated = false;
	end
end