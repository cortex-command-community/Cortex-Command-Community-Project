-- Incorporates Filipawn Industries code

function Create(self)
	self.turnStrength = 3;
	self.shake = 0.5;
	self.lifeTimer = Timer();
	
	if self:NumberValueExists("TargetID") and self:GetNumberValue("TargetID") ~= rte.NoMOID then
		local mo = MovableMan:GetMOFromID(self:GetNumberValue("TargetID"));
		if mo and IsActor(mo) then
			self.target = ToActor(mo);
			self.targetPos = mo.Pos;
			
			local dif = SceneMan:ShortestDistance(self.Pos,self.targetPos,SceneMan.SceneWrapsX);
			
			local angToTarget = dif.AbsRadAngle
			-- rotate towards where we wanna go preemptively
			
			local min_value = -math.pi;
			local max_value = math.pi;
			local value = angToTarget - self.RotAngle
			local result;
			
			local range = max_value - min_value;
			if range <= 0 then
				result = min_value;
			else
				local ret = (value - min_value) % range;
				if ret < 0 then ret = ret + range end
				result = ret + min_value;
			end
			
			self.AngularVel = math.min(15, math.max(-15, self.AngularVel * 0.1 + value * 15))
			
		end
	end
	
	self.lifeTimer:SetSimTimeLimitMS(math.random(self.Lifetime * 0.5, self.Lifetime - math.ceil(TimerMan.DeltaTimeMS)));
	self.activationDelay = math.random(260, 300);
end

function Update(self)
	self.GlobalAccScalar = 1/math.sqrt(1 + math.abs(self.Vel.X) * 0.1);
	if self.lifeTimer:IsPastSimMS(self.activationDelay) then
		if self:IsEmitting() == false then
			self:EnableEmission(true);
		end
		if self.target then
			local dif = SceneMan:ShortestDistance(self.Pos,self.targetPos,SceneMan.SceneWrapsX);
			
			local angToTarget = dif.AbsRadAngle
			
			local velCurrent = self.Vel-- + SceneMan.GlobalAcc
			local velTarget = Vector(100, 0):RadRotate(angToTarget)
			local velDif = velTarget - velCurrent
	
			-- Frotate self.hoverDirection
			local min_value = -math.pi;
			local max_value = math.pi;
			local value = velDif.AbsRadAngle - self.RotAngle
			local result;
			
			local range = max_value - min_value;
			if range <= 0 then
				result = min_value;
			else
				local ret = (value - min_value) % range;
				if ret < 0 then ret = ret + range end
				result = ret + min_value;
			end
			
			self.RotAngle = self.RotAngle + result * TimerMan.DeltaTimeSecs * 45
			
			-- acceleration
			self.Vel = self.Vel + Vector(math.pow(math.min(velDif.Magnitude, 25), 1.5), 0):RadRotate(self.RotAngle) * TimerMan.DeltaTimeSecs
		end

	end
end