function Create(self)

	self.turnStrength = 3;
	self.shake = 0.5;
	self.lifeTimer = Timer();

	if self:NumberValueExists("TargetID") and self:GetNumberValue("TargetID") ~= rte.NoMOID then
		local mo = MovableMan:GetMOFromID(self:GetNumberValue("TargetID"));
		if mo and IsActor(mo) then
			self.target = ToActor(mo);
		end
	end
	self.lifeTimer:SetSimTimeLimitMS(math.random(self.Lifetime * 0.5, self.Lifetime - math.ceil(TimerMan.DeltaTimeMS)));
	self.activationDelay = math.random(50, 100);
end

function Update(self)
	self.GlobalAccScalar = 1/math.sqrt(1 + math.abs(self.Vel.X) * 0.1);
	if self.lifeTimer:IsPastSimMS(self.activationDelay) then
		self:EnableEmission(true);
		if self.target and self.target.ID ~= rte.NoMOID then
			local targetDist = SceneMan:ShortestDistance(self.Pos, self.target.Pos, SceneMan.SceneWrapsX);
			if targetDist:MagnitudeIsLessThan(self.Radius + self.target.IndividualRadius) then
				self:GibThis();
			else
				local targetVel = targetDist:SetMagnitude(self.turnStrength);

				local turnAngle = self.Vel.AbsRadAngle - targetVel.AbsRadAngle;
				turnAngle = turnAngle > math.pi and turnAngle - (math.pi * 2) or (turnAngle < -math.pi and turnAngle + (math.pi * 2) or turnAngle);

				self.Vel = (self.Vel + targetVel):SetMagnitude(self.Vel.Magnitude);
				self.AngularVel = self.AngularVel * 0.5 - (turnAngle * self.turnStrength);
			end
		end
		self.Vel = Vector(self.Vel.X, self.Vel.Y):DegRotate(RangeRand(-self.shake, self.shake) * math.sqrt(1 + self.Vel.Magnitude));
		if self.lifeTimer:IsPastSimTimeLimit() or (self.Vel.Magnitude + math.abs(self.AngularVel)) < 3 then
			self:GibThis();
		end
	end
end